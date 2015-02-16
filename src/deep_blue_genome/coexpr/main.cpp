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
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneCorrelationMatrix.h>
#include <deep_blue_genome/coexpr/Baits.h>
#include <yaml-cpp/yaml.h>

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

	bool operator<(const BaitCorrelation& other) const {
		return bait < other.bait;
	}

private:
	Gene bait;
	double correlation; // correlation to owner
};

const Gene& BaitCorrelation::get_bait() const {
	return bait;
}

double BaitCorrelation::get_correlation() const {
	return correlation;
}

namespace std {
template <> struct hash<BaitCorrelation>
{
	size_t operator()(const BaitCorrelation& x) const
	{
		size_t hash = 0;
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_bait());
		return hash;
	}
};
} // end namespace

/**
 * A group of orthologs
 */
class OrthologGroup : public boost::noncopyable
{
public:
	/**
	 * @param genes Iterable of distinct genes
	 */
	template <class GeneCollectionsIterable>
	OrthologGroup(OrthologGroupId group_id, const GeneCollectionsIterable& all_gene_collections, Database& database)
	{
		auto genes = database.get_orthologs(group_id, all_gene_collections);
		this->genes.reserve(genes.size());
		for (auto& gene_id : genes) {
			this->genes.emplace_back(database.get_gene(gene_id));
			name += this->genes.back().get_name() + ";";
		}
	}

	bool operator==(const OrthologGroup& other) const = delete;

	string get_name() const;

	void add_bait_correlation(const Gene& bait, double correlation) {
		bait_correlations.emplace(bait, correlation);
	}

	const set<BaitCorrelation>& get_bait_correlations() {
		return bait_correlations;
	}

	vector<Gene>::const_iterator begin() const;
	vector<Gene>::const_iterator end() const;

private:
	vector<Gene> genes;
	set<BaitCorrelation> bait_correlations;  // TODO why ordered?
	string name;
};

vector<Gene>::const_iterator OrthologGroup::begin() const {
	return genes.begin();
}

vector<Gene>::const_iterator OrthologGroup::end() const {
	return genes.end();
}

string OrthologGroup::get_name() const {
	return name;
}

/**
 * Cache of ortholog groups
 */
class OrthologGroups
{
public:
	template <class GeneCollectionsIterable>
	OrthologGroups(Database& database, const GeneCollectionsIterable& all_gene_collections)
	:	database(database), all_gene_collections(all_gene_collections.begin(), all_gene_collections.end())
	{
	}

	/**
	 * Get Group of gene
	 */
	OrthologGroup& get(const Gene& gene) {
		auto group_id = gene.get_ortholog_group_id();
		auto it = groups.find(group_id);
		if (it == groups.end()) {
			// make group
			auto p = groups.emplace(piecewise_construct,
					forward_as_tuple(group_id),
					forward_as_tuple(group_id, all_gene_collections, database)
			);
			return p.first->second;
		}
		else {
			return it->second;
		}
	}

private:
	Database& database;
	vector<GeneCollectionId> all_gene_collections;
	unordered_map<OrthologGroupId, OrthologGroup> groups;
};

// TODO refactor into functions rather than many file sections
int main(int argc, char** argv) {
	using namespace DEEP_BLUE_GENOME;
	using namespace DEEP_BLUE_GENOME::COEXPR;
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

		// Read yaml
		YAML::Node job_node = YAML::LoadFile(argv[1]);
		string baits_path = job_node["baits"].as<string>();

		double negative_treshold = job_node["negative_treshold"].as<double>();
		ensure(fabs(negative_treshold) <= 1.0+1e-7, "negative_treshold must be a double between -1 and 1", ErrorType::GENERIC);

		double positive_treshold = job_node["positive_treshold"].as<double>();
		ensure(fabs(positive_treshold) <= 1.0+1e-7, "positive_treshold must be a double between -1 and 1", ErrorType::GENERIC);

		unordered_set<GeneCollectionId> all_gene_collections; // all collections needed for this run
		map<GeneCollectionId, shared_ptr<GeneExpressionMatrix>> expression_matrices; // gene collection -> matrix
		for (auto matrix_node : job_node["expression_matrices"]) {
			GeneCollectionId gene_collection = database.get_gene_collection_id(matrix_node["gene_collection"].as<string>());
			string matrix_name = matrix_node["name"].as<string>();

			bool created = all_gene_collections.emplace(gene_collection).second;
			ensure(created, "Specified multiple matrices of the same gene collection", ErrorType::GENERIC);

			auto matrix_id = database.get_gene_expression_matrix_id(gene_collection, matrix_name);
			expression_matrices.emplace(gene_collection, database.get_gene_expression_matrix(matrix_id));
		}

		// Gene groups
		OrthologGroups groups(database, all_gene_collections);

		// Load baits
		vector<OrthologGroup*> baits; // list of distinct baits
		{
			Baits baits_(baits_path);
			for (const auto& gene_name : baits_.get_genes()) {
				GeneVariant gene_variant = database.get_gene_variant(gene_name);

				ensure(!gene_variant.is_splice_variant() || gene_variant.get_splice_variant_id() == 1, // expect genes, but be lenient by accepting first splice variant as entire gene
						(make_string() << "Baits must be genes, got splice variant instead: " << gene_name).str(),
						ErrorType::GENERIC
				);

				// if a member is missing from its corresponding expression matrix, drop the bait
				auto& group = groups.get(gene_variant.get_gene());
				for (auto& gene : group) {
					if (!expression_matrices.at(gene.get_gene_collection_id())->has_gene(gene.get_id())) {
						cout << "Warning: Dropped bait gene '" << gene_name << "' due to: missing in expression matrix." << "\n";
						continue;
					}
				}

				baits.emplace_back(&group);
			}
		}
		sort(baits.begin(), baits.end());
		unique(baits.begin(), baits.end());

		// If baits are dropped, gene collections might drop out too, so we rebuild all_gene_collections here
		all_gene_collections.clear();
		for (auto group : baits) {
			for (auto& bait : *group) {
				all_gene_collections.emplace(bait.get_gene_collection_id());
			}
		}

		cout.flush();

		// Grab union of neighbours of each bait, where neighbour relation is sufficient (anti-)correlation
		std::vector<OrthologGroup*> neighbours;
		unordered_map<GeneCollectionId, vector<size_type>> bait_indices; // species -> gene indices of baits
		for (auto group : baits) {
			for (auto bait : *group) {
				// Calculate correlations
				auto matrix = expression_matrices.at(bait.get_gene_collection_id());
				bait_indices[bait.get_gene_collection_id()].emplace_back(matrix->get_gene_row(bait.get_id()));
			}
		}
		for (auto& gene_collection_id : all_gene_collections) {
			auto expression_matrix = expression_matrices.at(gene_collection_id);
			auto& indices = bait_indices.at(gene_collection_id);

			GeneCorrelationMatrix correlations(*expression_matrix, indices);
			auto& correlations_ = correlations.get();

			// Make edges to nodes with sufficient correlation
			for (auto row_index : indices) {
				auto col_index = correlations.get_column_index(row_index);
				auto bait_id = expression_matrix->get_gene_id(row_index);
				auto bait = database.get_gene(bait_id);
				for (size_type gene = 0; gene < correlations_.size1() && gene != row_index; gene++) {
					auto corr = correlations_(gene, col_index);
					if (corr < negative_treshold || corr > positive_treshold) {
						auto row_gene = database.get_gene(expression_matrix->get_gene_id(gene));
						auto& group = groups.get(row_gene);
						group.add_bait_correlation(bait, corr);
						neighbours.emplace_back(&group);
					}
				}
			}
		}

		sort(neighbours.begin(), neighbours.end());
		unique(neighbours.begin(), neighbours.end());

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

		for (auto group : baits) {
			for (auto bait : *group) {
				out_sif << bait.get_name() << "\n";
				out_node_attr << bait.get_name() << " = Bait\n";
			}
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
