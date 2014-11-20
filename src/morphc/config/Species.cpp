// Author: Tim Diels <timdiels.m@gmail.com>

#include "Species.h"
#include <morphc/GeneExpression.h>
#include <morphc/Clustering.h>
#include <morphc/Ranking.h>
#include <morphc/util.h>

using namespace std;
using namespace MORPHC;

namespace MORPHC {
namespace CONFIG {

Species::Species(string parent_data_root, YAML::Node node)
:	name(node["name"].as<string>())
{
	string data_root = prepend_path(parent_data_root, node["data_path"].as<string>("."));

	for (auto expression_matrix : node["expression_matrices"]) {
		gene_expressions.emplace_back(data_root, expression_matrix);
	}
}

void Species::add_job(std::string data_root, YAML::Node node) {
	genes_of_interest_sets.emplace_back(data_root, node);
}

void Species::run_jobs(string output_path) {
	if (genes_of_interest_sets.empty())
		return;

	for (auto& gene_expression_ : gene_expressions) {
		MORPHC::GeneExpression gene_expression(gene_expression_.get_path());

		// translate gene names to indices; and drop genes missing from the gene expression data
		std::vector<std::vector<size_type>> goi_sets;
		for (auto& goi : genes_of_interest_sets) {
			goi_sets.emplace_back();
			for (auto gene : goi.get_genes()) {
				if (gene_expression.has_gene(gene)) {
					goi_sets.back().emplace_back(gene_expression.get_gene_index(gene));
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

		gene_expression.generate_gene_correlations(all_goi);

		// clustering
		for (auto clustering_ : gene_expression_.get_clusterings()) {
			MORPHC::Clustering clustering(clustering_, gene_expression);
			int goi_index=0;
			for (int i=0; i < goi_sets.size(); i++) {
				auto& goi = goi_sets.at(i);
				string name = (make_string() << get_name() << "__" << genes_of_interest_sets.at(i).get_name() << "__" << gene_expression_.get_name() << "__" << clustering.get_name() << ".txt").str();
				replace(begin(name), end(name), ' ', '_');
				cout << "Current job: " << name << endl;
				if (goi.empty()) {
					throw runtime_error("Not implemented");// TODO return empty result with warning
				}
				else {
					// Rank genes
					Ranking ranking(goi, clustering);
					ranking.save(output_path + "/" + name);
				}
				goi_index++;
			}
		}
	}
}

string Species::get_name() const {
	return name;
}

}}
