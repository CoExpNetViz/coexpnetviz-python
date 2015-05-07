/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Species.h"
#include <iomanip>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneCorrelationMatrix.h>
#include <deep_blue_genome/common/GeneExpressionMatrixClustering.h>
#include <deep_blue_genome/common/Canonicaliser.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/morph/GOIResult.h>
#include <deep_blue_genome/morph/GenesOfInterest.h>

using namespace std;

namespace DEEP_BLUE_GENOME {
namespace MORPH {

Species::Species(std::string name, Database& database)
:	species(database.get_species(name)), database(database)
{
}

void Species::add_goi(std::string name, std::string path) {
	gois.emplace_back(name, path);
}

void Species::run_jobs(string output_path, int top_k, bool output_yaml) {
	if (gois.empty())
		return;

	// Load gene mapping
	Canonicaliser canonicaliser(database, get_name());

	// Load gois
	std::vector<GenesOfInterest> gois;
	for (auto& p : this->gois) {
		gois.emplace_back(p.first, p.second, species->get_gene_pattern_re());
		auto& goi = gois.back();
		goi.canonicalise(canonicaliser);

		auto size = goi.get_genes().size();
		if (size < 5) {
			cout << "Dropping GOI " << goi.get_name() << ": too few genes: " << size << " < 5\n";
			gois.pop_back();
		}
	}

	// For each GeneExpression, ge.Cluster, GOI calculate the ranking and keep the best one per GOI
	map<int, GOIResult> results; // goi index -> result for goi
	for (auto gem_name : species->get_gene_expression_matrices()) {
		auto gene_expression = species->get_gene_expression_matrix(gem_name);

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
		unique_ptr<DEEP_BLUE_GENOME::GeneCorrelationMatrix> gene_correlations;
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
			gene_correlations = make_unique<DEEP_BLUE_GENOME::GeneCorrelationMatrix>(*gene_expression, all_genes_of_interest);
			gene_expression->dispose_expression_data();
		}

		// clustering
		for (auto clustering_name : gene_expression->get_clusterings()) {
			auto clustering = gene_expression->get_clustering(clustering_name);
			int goi_index=0;
			for (int i=0; i < gois_indices.size(); i++) {
				auto& goi = gois_indices.at(i);
				cout << get_name() << ", " << gois.at(i).get_name() << ", " << gene_expression->get_name() << ", " << clustering->get_name();
				cout.flush();
				if (goi.size() < 5) {
					cout << ": Skipping: Too few genes of interest found in dataset: " << goi.size() << " < 5\n";
					continue;
				}
				else {
					// Rank genes
					string name = (make_string() << get_name() << "__" << gois.at(i).get_name() << ".txt").str();
					replace(begin(name), end(name), ' ', '_');
					auto ranking = make_unique<Ranking>(goi, clustering, *gene_correlations, name);
					cout << ": AUSR=" << setprecision(2) << fixed << ranking->get_ausr() << "\n";

					auto result_it = results.find(i);
					if (result_it == results.end()) {
						result_it = results.emplace(piecewise_construct, make_tuple(i), make_tuple()).first;
					}

					auto& result = result_it->second;
					result.add_ausr(ranking->get_ausr());
					if (!result.best_ranking.get() || *ranking > *result.best_ranking) {
						result.best_ranking = std::move(ranking);
					}
				}
				goi_index++;
			}
		}
	}

	auto gene_descriptions = species->get_gene_descriptions(); // Keep descriptions loaded during the loop below
	for (auto& p : results) {
		auto& result = p.second;
		result.best_ranking->save(output_path, top_k, *species, gois.at(p.first), result.get_average_ausr(), output_yaml);
	}
}

string Species::get_name() const {
	return species->get_name();
}

}}
