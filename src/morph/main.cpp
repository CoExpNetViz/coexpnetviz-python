// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>

#include <string>
#include <vector>
#include <map>
#include <utility>

#include "Clustering.h"
#include "ublas.h"
#include "util.h"

#ifdef _WIN32

#else
#endif

// TODO fiddling with matrix orientation, would it help performance?

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

class Application
{
public:
	void run();

private:
	void load();

private:
	std::map<std::string, GeneExpression> gene_expression_sets; // source path -> GeneExpression
	std::vector<Clustering> clusterings;
	std::vector<GenesOfInterest> genes_of_interest_sets;
};

#include <fstream>

void Application::run() {
	load();

	// TODO actual calc
	/*map<GenesOfInterest*, std::vector<Ranking>> result_sets; // name of genes of interest set -> its rankings
	//results.reserve(clusterings.size());*/

	/* TODO genes from goi and gene_expression not present in clustering should be ignored in ranking
	// -> GeneExpression: make expr_matrix for all genes, make corr for genes X goi_genes [DONE]
	// -> Clustering: nothing special [DONE]
	// -> Ranking: [TODO put in inner loop here]
	 *    - use submatrix from expr_matrix having only gene_expression.genes intersect clustering.genes
	 *    - for each goi_set, remove missing genes
	 */
	for (auto& clustering : clusterings) {
		for (auto& genes_of_interest : genes_of_interest_sets) {
			//clustering.get_genes();
			//genes_of_interest.get
			//Ranking ranking(genes_of_interest, clustering);
			//results.emplace_back(ranking);
		}
	}

	// TODO print results
}

void Application::load() {
	// Load genes of interest sets
	std::vector<string> goi_paths = {"../data/Configs/InputTextGOI2.txt", "../data/Configs/InputTextGOI3.txt"};
	std::vector<string> all_genes_of_interest;
	for (string path : goi_paths) {
		genes_of_interest_sets.emplace_back(path);
		auto& goi = genes_of_interest_sets.back().get_genes();
		all_genes_of_interest.insert(all_genes_of_interest.end(), goi.begin(), goi.end());
	}
	sort(all_genes_of_interest.begin(), all_genes_of_interest.end());
	all_genes_of_interest.erase(unique(all_genes_of_interest.begin(), all_genes_of_interest.end()), all_genes_of_interest.end());

	// Load config: lists clustering files and their associated gene expression files
	string config_path = "../data/Configs/ConfigsBench.txt";
	read_file(config_path, [this, &all_genes_of_interest](ifstream& in) {
		while (in.good()) {
			string gene_expression_path;
			string clustering_path;
			in >> gene_expression_path;
			if (in.fail()) {
				break; // probably eof
			}
			in >> clustering_path;
			if (in.fail()) {
				throw runtime_error("Incomplete line");
			}

			// TODO no hack
			gene_expression_path = "../data/" + gene_expression_path;
			clustering_path = "../data/" + gene_expression_path;

			auto it = gene_expression_sets.find(gene_expression_path);
			if (it == gene_expression_sets.end()) {
				it = gene_expression_sets.emplace(gene_expression_path, GeneExpression(gene_expression_path, all_genes_of_interest)).first;
			}

			clusterings.emplace_back(Clustering(clustering_path, it->second));

			throw runtime_error("dbg");
		}
	});
}

int main() {
	Application app;
	app.run();
	return 0;
}
