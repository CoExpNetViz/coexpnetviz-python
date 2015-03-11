// Author: Tim Diels <timdiels.m@gmail.com>

// Note: we work with orthologs, i.e. we work at the level of 'Gene's, not 'GeneVariant's

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <boost/filesystem.hpp> // TODO remove unused includes here
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/coexpr/Baits.h>
#include <deep_blue_genome/coexpr/BaitCorrelation.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfo.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfos.h>
#include <deep_blue_genome/coexpr/BaitGroups.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using namespace DEEP_BLUE_GENOME::COEXPR;


//////////////////////////
// Funcs

void read_yaml(std::string path, Database& database, string& baits_path, double& negative_treshold, double& positive_treshold, unique_ptr<OrthologGroupInfos>& groups, vector<GeneExpressionMatrix*>& expression_matrices) {
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

	groups = make_unique<OrthologGroupInfos>(gene_collections);
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

/**
 * Write out a cytoscape network
 */
void write_cytoscape_network(const vector<Gene*>& baits, const std::vector<OrthologGroupInfo*>& neighbours) {
	string install_dir = "/home/limyreth/doc/internship/deep_blue_genome"; // TODO don't hardcode, instead install_dir/... or something. with structure: ./bin, ./data; so dirname(argv[0])/..
	string network_name = "network1"; // TODO

	ofstream out_sif(network_name + ".sif");
	out_sif.exceptions(ofstream::failbit | ofstream::badbit);

	ofstream out_edge_attr(network_name + ".edge.attr");
	out_edge_attr.exceptions(ofstream::failbit | ofstream::badbit);
	out_edge_attr << "link\tR_value\n";

	ofstream out_node_attr(network_name + ".node.attr");
	out_node_attr.exceptions(ofstream::failbit | ofstream::badbit);
	out_node_attr << "Gene\tCorrelation_to_baits\tType\tNode_Information\tColor\tSpecies\tHomologs\n";

	// TODO copy vizmap, without using boost copy_file (or try with a new header)

	// output bait node attributes
	for (auto bait : baits) {
		out_node_attr << bait->get_name() << "\t"; // TODO this should be 'Id', meaning node id
		out_node_attr << "\t"; // Skip: correlations to bait
		out_node_attr << "Bait\t";
		out_node_attr << "\t"; // Skip: function annotation (TODO this column will be removed later)
		out_node_attr << "#FFFFFF\t";
		out_node_attr << bait->get_gene_collection().get_species() << "\t";
		// Skip: homologs
		out_node_attr << "\n";
	}

	// bait homologs TODO this would only make sense if actually adding homologs of baits
	/*for (auto bait : baits) {
		OrthologGroup* group = bait.get_ortholog_group();
		if (group) {
			out_edge_attr << bait->get_name() << "\t";
			out_node_attr << "\t";
			out_node_attr << "Bait\t";
			out_node_attr << "\n";
		}
	}*/

	// have each neigh figure out what its bait group is
	BaitGroups bait_groups;
	for (auto neigh : neighbours) {
		neigh->init_bait_group(bait_groups);
	}

	// assign colours to bait groups
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, 0x00FFFFFF);
	for (auto& p : bait_groups) {
		auto& group = p.second;

		ostringstream str;
		int colour = distribution(generator);
		str << "#";
		str.width(6);
		str.fill('0');
		str << hex << colour;

		group.set_colour(str.str());
	}

	// output neighbours
	for (auto neigh : neighbours) {
		// edge attr and sif
		if (!neigh->get_bait_correlations().empty()) {
			out_sif << neigh->get_name() << "\tcor";
			for (auto& bait_correlation : neigh->get_bait_correlations()) {
				auto bait_name = bait_correlation.get_bait().get_name();
				out_sif << "\t" << bait_name;
				out_edge_attr << neigh->get_name() << " (cor) " << bait_name << "\t" << bait_correlation.get_correlation() << "\n";
			}
			out_sif << "\n";
		}

		// node attr
		out_node_attr << neigh->get_name() << "\t";

		{
			bool first = true;
			for (auto& bait_correlation : neigh->get_bait_correlations()) {
				if (!first) {
					out_node_attr << " ";
				}
				out_node_attr << bait_correlation.get_bait().get_name();
				first = false;
			}
			out_node_attr << "\t";
		}

		out_node_attr << "Target\t";
		out_node_attr << "\t"; // Skipping func annotation for now (col will be removed later)
		out_node_attr << neigh->get_bait_group().get_colour() << "\t";
		out_node_attr << "\t"; // Skip species, these are of multiple
		out_node_attr << "\n";
	}
}

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
		unique_ptr<OrthologGroupInfos> groups;
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
		neighbours.erase(unique(neighbours.begin(), neighbours.end()), neighbours.end());

		// TODO remove baits with no edges (these can be from gene collections we didn't even check)

		// Output cytoscape files
		write_cytoscape_network(baits, neighbours);
	});
}
