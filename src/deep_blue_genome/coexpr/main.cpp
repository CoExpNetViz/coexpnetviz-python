// Author: Tim Diels <timdiels.m@gmail.com>

// TODO gene collection, expression matrix, ... names should be stored with case but matched without it

// Note: we work with orthologs, i.e. we work at the level of 'Gene's, not 'GeneVariant's

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/coexpr/Baits.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

/**
 * Correlation to a bait gene
 *
 * Data class of Group.
 */
class BaitCorrelation {
public:
	BaitCorrelation(const Gene& bait, double correlation)
	:	bait(bait), correlation(correlation)
	{
	}

	const Gene& get_bait() const;
	double get_correlation() const;

private:
	const Gene& bait;
	double correlation; // correlation to owner
};

const Gene& BaitCorrelation::get_bait() const {
	return bait;
}

double BaitCorrelation::get_correlation() const {
	return correlation;
}

/**
 * A group of orthologs
 */
class OrthologGroupInfo : public boost::noncopyable
{
public:
	/**
	 * @param genes Iterable of distinct genes
	 */
	OrthologGroupInfo(OrthologGroup& group, const vector<GeneCollection*>& gene_collections)
	:	group(group)
	{
		for (auto& gene : group) {
			name += gene->get_name() + ";";
		}
	}

	bool operator==(const OrthologGroup& other) const = delete;

	string get_name() const;

	void add_bait_correlation(const Gene& bait, double correlation);

	const vector<BaitCorrelation>& get_bait_correlations() const;

	vector<Gene*>::const_iterator begin() const;
	vector<Gene*>::const_iterator end() const;

private:
	OrthologGroup& group;
	vector<BaitCorrelation> bait_correlations;
	string name;
};

vector<Gene*>::const_iterator OrthologGroupInfo::begin() const {
	return group.begin();
}

vector<Gene*>::const_iterator OrthologGroupInfo::end() const {
	return group.end();
}

string OrthologGroupInfo::get_name() const {
	return name;
}

void OrthologGroupInfo::add_bait_correlation(const Gene& bait, double correlation) {
	auto match_bait = [&bait](const BaitCorrelation& bait_correlation) {
		return &bait_correlation.get_bait() == &bait;
	};

	if (find_if(bait_correlations.begin(), bait_correlations.end(), match_bait) != bait_correlations.end())
		return; // TODO should this even be allowed?

	bait_correlations.emplace_back(bait, correlation);
}

const vector<BaitCorrelation>& OrthologGroupInfo::get_bait_correlations() const {
	return bait_correlations;
}


class OrthologGroups
{
public:
	typedef vector<GeneCollection*> GeneCollections;

	OrthologGroups(GeneCollections gene_collections)
	:	gene_collections(std::move(gene_collections))
	{
	}

	/**
	 * Get Group of gene
	 */
	OrthologGroupInfo* get(const Gene& gene) {
		auto group = gene.get_ortholog_group();

		if (!group) {
			return nullptr;
		}

		auto it = groups.find(group);
		if (it == groups.end()) {
			// make group
			auto p = groups.emplace(piecewise_construct,
					forward_as_tuple(group),
					forward_as_tuple(*group, gene_collections)
			);
			return &p.first->second;
		}
		else {
			return &it->second;
		}
	}

private:
	GeneCollections gene_collections;
	unordered_map<OrthologGroup*, OrthologGroupInfo> groups;
};


//////////////////////////
// Funcs

using namespace DEEP_BLUE_GENOME;
using namespace DEEP_BLUE_GENOME::COEXPR;

void read_yaml(std::string path, Database& database, string& baits_path, double& negative_treshold, double& positive_treshold, unique_ptr<OrthologGroups>& groups, vector<GeneExpressionMatrix*>& expression_matrices) {
	YAML::Node job_node = YAML::LoadFile(path);
	baits_path = job_node["baits"].as<string>();

	negative_treshold = job_node["negative_treshold"].as<double>();
	ensure(fabs(negative_treshold) <= 1.0+1e-7, "negative_treshold must be a double between -1 and 1", ErrorType::GENERIC);

	positive_treshold = job_node["positive_treshold"].as<double>();
	ensure(fabs(positive_treshold) <= 1.0+1e-7, "positive_treshold must be a double between -1 and 1", ErrorType::GENERIC);

	DataFileImport importer(database);

	vector<GeneCollection*> gene_collections;
	int i=0;
	for (auto matrix_node : job_node["expression_matrices"]) {
		string matrix_path = matrix_node.as<string>();
		string matrix_name = "tmp:matrix" + (i++);
		auto& matrix = importer.add_gene_expression_matrix(matrix_name, matrix_path);

		auto& gene_collection = matrix.get_gene_collection();
		ensure(!contains(gene_collections, &gene_collection),
				"Specified multiple matrices of the same gene collection",
				ErrorType::GENERIC
		);
		gene_collections.emplace_back(&gene_collection);

		expression_matrices.emplace_back(&matrix);
	}

	groups = make_unique<OrthologGroups>(gene_collections);
}

/**
 * Load baits as distinct list of groups
 */
std::vector<Gene*> load_baits(Database& database, std::string baits_path) {
	vector<Gene*> baits;

	// read file
	Baits baits_(baits_path);
	for (const auto& gene_name : baits_.get_genes()) {
		auto& gene_variant = database.get_gene_variant(gene_name);
		auto& gene = gene_variant.as_gene();
		baits.emplace_back(&gene);
	}

	// get rid of duplicates in input
	sort(baits.begin(), baits.end());
	unique(baits.begin(), baits.end());

	return baits;
}

// TODO refactor into functions rather than many file sections
int main(int argc, char** argv) {
	// TODO could merging of ortho groups be done badly? Look at DataFileImport
	// TODO allow custom orthologs files
	graceful_main([argc, argv](){
		// Read args
		if (argc != 2) {
			cout
				<< "Usage: coexpr yaml_file\n"
				<< "\n"
				<< "- yaml_file: path to file in yaml format with description of what to calculate\n"
				<< endl;

			ensure(false, "Invalid argument count", ErrorType::GENERIC);
		}

		// Load database
		Database database;

		// Read input
		string baits_path;
		double negative_treshold;
		double positive_treshold;
		unique_ptr<OrthologGroups> groups;
		vector<GeneExpressionMatrix*> expression_matrices;
		read_yaml(argv[1], database, baits_path, negative_treshold, positive_treshold, groups, expression_matrices);

		vector<Gene*> baits = load_baits(database, baits_path);

		cout.flush();

		// Helper function: get matrix by gene collection
		auto get_matrix = [&expression_matrices](GeneCollection& collection) -> GeneExpressionMatrix* {
			auto match_collection = [&collection] (GeneExpressionMatrix* matrix) {
				return &matrix->get_gene_collection() == &collection;
			};
			auto it = find_if(expression_matrices.begin(), expression_matrices.end(), match_collection);
			if (it == expression_matrices.end()) {
				return nullptr;
			}
			else {
				return *it;
			}
		};

		// Group bait genes by expression matrix
		unordered_map<GeneExpressionMatrix*, vector<GeneExpressionMatrixRow>> bait_indices;
		for (auto bait : baits) {
			auto matrix = get_matrix(bait->get_gene_collection());
			if (matrix && matrix->has_gene(*bait)) {
				bait_indices[matrix].emplace_back(matrix->get_gene_row(*bait));
			}
		}

		// Grab union of neighbours of each bait, where neighbour relation is sufficient (anti-)correlation
		std::vector<OrthologGroupInfo*> neighbours;
		for (auto expression_matrix : expression_matrices) {
			auto& indices = bait_indices.at(expression_matrix);
			GeneCorrelationMatrix correlations(*expression_matrix, indices);
			auto& correlations_ = correlations.get();

			// Make edges to nodes with sufficient correlation
			for (auto row_index : indices) {
				auto col_index = correlations.get_column_index(row_index);
				auto& bait = expression_matrix->get_gene(row_index);
				for (GeneExpressionMatrixRow row = 0; row < correlations_.size1(); row++) {
					if (row == row_index) {
						continue; // Don't make edge from bait gene to itself
					}

					auto corr = correlations_(row, col_index);
					if (corr < negative_treshold || corr > positive_treshold) {
						auto& gene = expression_matrix->get_gene(row);
						auto group = groups->get(gene);
						if (group) { // Note: if gene has no orthologs, don't return it in the output. We assume it won't be interesting and would just add clutter
							group->add_bait_correlation(bait, corr);
							neighbours.emplace_back(group);
						}
					}
				}
			}
		}

		sort(neighbours.begin(), neighbours.end());
		unique(neighbours.begin(), neighbours.end());

		// TODO remove baits with no edges (these can be from gene collections we didn't even check)

		//////////////////////
		// Output cytoscape files using: baits, neighbours

		string install_dir = "/home/limyreth/doc/internship/deep_blue_genome"; // TODO don't hardcode, instead install_dir/... or something. with structure: ./bin, ./data; so dirname(argv[0])/..
		string network_name = "network1"; // TODO

		ofstream out_sif(network_name + ".sif");
		out_sif.exceptions(ofstream::failbit | ofstream::badbit);

		ofstream out_edge_attr(network_name + ".eda");
		out_edge_attr.exceptions(ofstream::failbit | ofstream::badbit);
		out_edge_attr << "R_VALUE (class=Double)" << "\n";

		ofstream out_node_attr(network_name + ".noa");
		out_node_attr.exceptions(ofstream::failbit | ofstream::badbit);
		out_node_attr << "Node_Information (class=java.lang.String)" << "\n";

		auto vizmap = network_name + ".props";
		boost::filesystem::copy_file(install_dir + "/data/templates/coexpr_vizmap.props", vizmap, boost::filesystem::copy_option::overwrite_if_exists);
		ofstream out_vizmap(vizmap, ios::app);
		out_vizmap.exceptions(ofstream::failbit | ofstream::badbit);

		for (auto bait : baits) {
			out_sif << bait->get_name() << "\n"; // TODO don't print this and don't output baits that correlate with nothing (although that's a really edgy edge case)
			out_node_attr << bait->get_name() << " = Bait\n";
		}

		unordered_set<string> bait_groups;
		for (auto neigh : neighbours) {
			string bait_group_name;
			for (auto& bait_correlation : neigh->get_bait_correlations()) {
				auto bait_name = bait_correlation.get_bait().get_name();
				bait_group_name += bait_name + ";";
				out_sif << bait_name << "\tpd\t" << neigh->get_name() << "\n";
				out_edge_attr << bait_name << " pd " << neigh->get_name() << " = " << bait_correlation.get_correlation() << "\n";
			}
			bait_groups.emplace(bait_group_name);
			out_node_attr << neigh->get_name() << " = " << bait_group_name << "\n";
		}

		// Pick size(owner_groups) colors
		// TODO include in thesis: alg for having N maximally orthogonal colours (though it's not a real fancy alg or idea...). We did the math on paper, translate it to latex
		//colors;

		for (auto& group_name : bait_groups) {
			// TODO color more cleverly, perhaps this helps: http://www.nps.edu/faculty/olsen/Remote_sensing/SPIE_2000_tyo_display.pdf
			out_vizmap << "nodeFillColorCalculator.default-Node\\ Color-Discrete\\ Mapper.mapping.map." << group_name << "=" << rand() % 256 << "," << rand() % 256 << "," << rand() % 256 << "\n";

			// size
			out_vizmap << "nodeUniformSizeCalculator.default-Node\\ Size-Discrete\\ Mapper.mapping.map." << group_name << "=80.0\n";
		}
	});
}
