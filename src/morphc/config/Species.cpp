// Author: Tim Diels <timdiels.m@gmail.com>

#include "Species.h"
#include <morphc/GeneExpression.h>
#include <morphc/Clustering.h>
#include <morphc/Ranking.h>
#include <morphc/util.h>
#include <iomanip>

using namespace std;
using namespace MORPHC;

namespace MORPHC {
namespace CONFIG {

Species::Species(string parent_data_root, YAML::Node node)
:	name(node["name"].as<string>()), gene_webpage_template(node["gene_web_page"].as<string>())
{
	string data_root = prepend_path(parent_data_root, node["data_path"].as<string>("."));
	gene_descriptions_path = prepend_path(data_root, node["gene_descriptions"].as<string>());

	for (auto expression_matrix : node["expression_matrices"]) {
		gene_expressions.emplace_back(data_root, expression_matrix);
	}
}

void Species::add_job(std::string data_root, YAML::Node node) {
	genes_of_interest_sets.emplace_back(data_root, node);
}

void Species::run_jobs(string output_path, int top_k) {
	if (genes_of_interest_sets.empty())
		return;

	map<int, unique_ptr<Ranking>> best_ranking_by_goi; // goi index -> best ranking
	for (auto& gene_expression_ : gene_expressions) {
		auto gene_expression = make_shared<MORPHC::GeneExpression>(gene_expression_.get_path());

		// translate gene names to indices; and drop genes missing from the gene expression data
		std::vector<std::vector<size_type>> goi_sets;
		for (auto& goi : genes_of_interest_sets) {
			goi_sets.emplace_back();
			for (auto gene : goi.get_genes()) {
				if (gene_expression->has_gene(gene)) {
					goi_sets.back().emplace_back(gene_expression->get_gene_index(gene));
				}
			}
		}

		// distinct union all left over genes of interest
		std::vector<size_type> all_goi;
		for (auto& goi : goi_sets) {
			all_goi.insert(all_goi.end(), goi.begin(), goi.end());
		}
		sort(all_goi.begin(), all_goi.end());
		all_goi.erase(unique(all_goi.begin(), all_goi.end()), all_goi.end());

		gene_expression->generate_gene_correlations(all_goi);

		// clustering
		for (auto clustering_ : gene_expression_.get_clusterings()) {
			auto clustering = make_shared<MORPHC::Clustering>(clustering_, gene_expression);
			int goi_index=0;
			for (int i=0; i < goi_sets.size(); i++) {
				auto& goi = goi_sets.at(i);
				cout << get_name() << ", " << genes_of_interest_sets.at(i).get_name() << ", " << gene_expression_.get_name() << ", " << clustering->get_name();
				cout.flush();
				if (goi.empty()) {
					cout << "Skipping: None of the gene of interests are in the dataset" << endl;
					throw runtime_error("Encountered empty GOI");// TODO this could legitimately occur when specifying a GOI of genes that don't appear in the gene-expression dataset.
				}
				else {
					// Rank genes
					string name = (make_string() << get_name() << "__" << genes_of_interest_sets.at(i).get_name() << ".txt").str();
					replace(begin(name), end(name), ' ', '_');
					auto ranking = make_unique<Ranking>(goi, clustering, name);
					cout << ": AUSR=" << setprecision(9) << fixed << ranking->get_ausr() << endl;
					if (ranking > best_ranking_by_goi[i])
						best_ranking_by_goi[i] = std::move(ranking);
				}
				goi_index++;
			}
		}
	}

	GeneDescriptions gene_descriptions(gene_descriptions_path);
	for (auto& p : best_ranking_by_goi) {
		p.second->save(output_path, top_k, gene_descriptions, gene_webpage_template);
	}
}

string Species::get_name() const {
	return name;
}

}}
