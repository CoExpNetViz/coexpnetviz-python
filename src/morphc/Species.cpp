// Author: Tim Diels <timdiels.m@gmail.com>

#include "Species.h"
#include <morphc/GeneExpression.h>
#include <morphc/Clustering.h>
#include <morphc/GeneMapping.h>
#include <morphc/Ranking.h>
#include <morphc/util.h>
#include "GenesOfInterest.h"
#include <iomanip>

using namespace std;

namespace MORPHC {

Species::Species(string parent_data_root, const YAML::Node node)
:	species(node)
{
	get_name();
	data_root = prepend_path(parent_data_root, node["data_path"].as<string>("."));
}

void Species::add_job(std::string data_root, const YAML::Node& node) { // TODO load later instead of right away
	gois.emplace_back(data_root, node);
}

void Species::run_jobs(string output_path, int top_k) {
	if (gois.empty())
		return;

	// Load gene mapping
	unique_ptr<GeneMapping> gene_mapping;
	if (species["gene_mapping"]) {
		gene_mapping = make_unique<GeneMapping>(prepend_path(data_root, species["gene_mapping"].as<string>()));
	}

	// Load gois
	std::vector<GenesOfInterest> gois;
	for (auto& p : this->gois) {
		gois.emplace_back(p.first, p.second);
		auto& goi = gois.back();
		if (gene_mapping.get()) {
			goi.apply_mapping(*gene_mapping);
		}
		auto size = goi.get_genes().size();
		if (size < 10) {
			cout << "Dropping GOI " << goi.get_name() << ": too few genes: " << size << " < 10\n";
			gois.pop_back();
		}
	}

	// For each GeneExpression, ge.Cluster, GOI calculate the ranking and keep the best one per GOI
	map<int, unique_ptr<Ranking>> best_ranking_by_goi; // goi index -> best ranking
	for (auto gene_expression_description : species["expression_matrices"]) {
		auto gene_expression = make_shared<GeneExpression>(data_root, gene_expression_description);

		// translate gene names to indices; and drop genes missing from the gene expression data
		std::vector<std::vector<size_type>> gois_indices; // gois, but specified by gene indices, not names
		for (int i=0; i < gois.size(); i++) {
			gois_indices.emplace_back();
			for (auto gene : gois.at(i).get_genes()) {
				if (gene_expression->has_gene(gene)) {
					gois_indices.back().emplace_back(gene_expression->get_gene_index(gene));
				}
			}
		}

		// generate correlations
		{
			// distinct union all left over genes of interest
			std::vector<size_type> all_genes_of_interest;
			for (auto& goi : gois_indices) {
				all_genes_of_interest.insert(all_genes_of_interest.end(), goi.begin(), goi.end());
			}
			sort(all_genes_of_interest.begin(), all_genes_of_interest.end());
			auto duplicate_begin = unique(all_genes_of_interest.begin(), all_genes_of_interest.end());
			all_genes_of_interest.erase(duplicate_begin, all_genes_of_interest.end());

			// generate the correlations we need
			gene_expression->generate_gene_correlations(all_genes_of_interest);
		}

		// clustering
		for (auto clustering_ : gene_expression_description["clusterings"]) {
			auto clustering = make_shared<Clustering>(gene_expression, data_root, clustering_);
			int goi_index=0;
			for (int i=0; i < gois_indices.size(); i++) {
				auto& goi = gois_indices.at(i);
				cout << get_name() << ", " << gois.at(i).get_name() << ", " << gene_expression->get_name() << ", " << clustering->get_name();
				cout.flush();
				if (goi.empty()) {
					cout << "Skipping: None of the gene of interests are in the dataset" << endl;
					throw runtime_error("Encountered empty GOI");// TODO this could legitimately occur when specifying a GOI of genes that don't appear in the gene-expression dataset.
				}
				else {
					// Rank genes
					string name = (make_string() << get_name() << "__" << gois.at(i).get_name() << ".txt").str();
					replace(begin(name), end(name), ' ', '_');
					auto ranking = make_unique<Ranking>(goi, clustering, name);
					cout << ": AUSR=" << setprecision(9) << fixed << ranking->get_ausr() << endl;
					if (!best_ranking_by_goi[i].get() || *ranking > *best_ranking_by_goi[i]) {
						best_ranking_by_goi[i] = std::move(ranking);
					}
				}
				goi_index++;
			}
		}
	}

	GeneDescriptions gene_descriptions(prepend_path(data_root, species["gene_descriptions"].as<string>()));
	for (auto& p : best_ranking_by_goi) {
		p.second->save(output_path, top_k, gene_descriptions, species["gene_web_page"].as<string>(), gois.at(p.first));
	}
}

string Species::get_name() const {
	return species["name"].as<string>();
}

}
