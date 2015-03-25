// Author: Tim Diels <timdiels.m@gmail.com>

// Vocab note:
// - bait gene: One of the genes provided by the user to which target genes are compared in terms of co-expression
// - target gene: any gene that's not a bait gene

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
#include <deep_blue_genome/coexpr/BaitCorrelations.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfo.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfos.h>
#include <deep_blue_genome/coexpr/CytoscapeWriter.h>

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

int main(int argc, char** argv) {
	// TODO could merging of ortho groups be done badly? Look at DataFileImport
	// TODO allow custom orthologs files
	graceful_main([argc, argv](){
		namespace fs = boost::filesystem;
		string install_dir = fs::canonical(fs::path(argv[0])).remove_filename().parent_path().native();

		// Read args
		if (argc != 3) {
			cout
				<< "Usage: coexpr database_path yaml_file\n"
				<< "\n"
				<< "- database_path: path to database directory created with the database command\n"
				<< "- yaml_file: path to file in yaml format with description of what to calculate\n"
				<< endl;

			ensure(false, "Invalid argument count", ErrorType::GENERIC);
		}

		// Load database
		Database database(argv[1]);

		// Read input
		string baits_path;
		double negative_treshold;
		double positive_treshold;
		unique_ptr<OrthologGroupInfos> groups;
		vector<GeneExpressionMatrix*> expression_matrices;
		read_yaml(argv[2], database, baits_path, negative_treshold, positive_treshold, groups, expression_matrices);

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
					if (contains(indices, row)) {
						continue; // Don't make edges between baits
					}

					auto corr = correlations_(row, col_index);
					if (corr < negative_treshold || corr > positive_treshold) {
						auto& gene = expression_matrix->get_gene(row);
						auto group = groups->get(gene);
						if (group) { // Note: if gene has no orthologs, don't return it in the output. We assume it won't be interesting and would just add clutter
							group->add_bait_correlation(gene, bait, corr);
							neighbours.emplace_back(group);
						}
					}
				}
			}
		}

		sort(neighbours.begin(), neighbours.end());
		neighbours.erase(unique(neighbours.begin(), neighbours.end()), neighbours.end());

		// Output cytoscape files
		CytoscapeWriter writer;
		writer.write(install_dir, baits, neighbours, groups.get());
	});
}
