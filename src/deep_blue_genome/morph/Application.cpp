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

#include <deep_blue_genome/morph/stdafx.h>
#include "Application.h"
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/ublas.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/morph/GOIResult.h>

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;
namespace fs = boost::filesystem;
using boost::container::flat_set;

namespace DEEP_BLUE_GENOME {
namespace MORPH {

Application::Application(int argc, char** argv)
{
	cout << setprecision(9);

	try {
		ensure(argc == 5 || argc == 6, "Invalid argument count", ErrorType::GENERIC);

		config_path = argv[1];
		job_list_path = argv[2];
		output_path = argv[3];
		istringstream str(argv[4]);
		str >> top_k;
		ensure(!str.fail(), "top_k argument must be an integer", ErrorType::GENERIC);
		ensure(top_k > 0, "top_k must be >0", ErrorType::GENERIC);
		if (argc == 6) {
			ensure(string(argv[5]) == "--output-yaml", "Invalid 5th argument", ErrorType::GENERIC);
			output_yaml = true;
		}
		else {
			output_yaml = false;
		}
	}
	catch (const exception& e) {
		cerr << "USAGE: morphc path/to/database.yaml path/to/joblist.yaml path/to/output_directory top_k [--output-yaml]\n"
			<< "\n"
			<< "top_k = max number of candidate genes to save in outputted rankings\n"
			<< "--output-yaml = when specified, rankings are saved in yaml format, otherwise they are saved in plain text format\n"
			<< "\n"
			<< "RETURN CODES:\n";
		std::array<std::string, 3> error_descriptions = {"No error", "Generic error", "GOI contains gene with invalid name"};
		for (int i=0; i<error_descriptions.size(); i++) {
			cerr << "  " << i << ": " << error_descriptions.at(i) << "\n";
		}
		cerr << "\n" << endl;
		throw;
	}
	// TODO look for asserts and see whether they should perhaps be needed at release as well
}

void Application::run() {
	load_config();
	load_jobs();
	run_jobs();
}

void Application::load_config() {
	database = make_unique<Database>("/home/limyreth/dbg_db"); // TODO from input
	// TODO s/config_path/database_path/ in args in.
}

void Application::load_jobs() {
	// Load jobs
	YAML::Node job_list = YAML::LoadFile(job_list_path);
	string data_root = job_list["data_path"].as<string>(".");

	for (auto job_group : job_list["jobs"]) {
		// TODO change joblist to be: path to a directory of goi, or path to a goi. Not a YAML file
		string goi_root = prepend_path(data_root, job_group["data_path"].as<string>("."));
		for (auto goi : job_group["genes_of_interest"]) {
			string name = goi["name"].as<string>();
			jobs.emplace(piecewise_construct, forward_as_tuple(name), forward_as_tuple(*database, name, prepend_path(goi_root, goi["path"].as<string>())));
		}
	}
}

void Application::run_jobs() {
	// For each GeneExpression, ge.Cluster, GOI calculate the ranking and keep the best one per GOI
	for (auto&& job : (jobs | boost::adaptors::map_values)) {
		GOIResult result;

		for (auto&& gene_expression : database->get_gene_expression_matrices()) {
			// translate bait references to their row index in the gene expression matrix (if any); this drops genes missing in the gene expression matrix
			flat_set<GeneExpressionMatrixRow> bait_indices;
			for (auto&& gene : job.get_genes()) {
				if (gene_expression.has_gene(gene)) {
					bait_indices.emplace(gene_expression.get_gene_row(gene));
				}
			}

			// generate correlations
			DEEP_BLUE_GENOME::GeneCorrelationMatrix gene_correlations(gene_expression, bait_indices);

			// clustering
			for (auto&& clustering : database->get_clusterings()) {
				try {
					GeneExpressionMatrixClustering clustering_(gene_expression, clustering);
					cout << job.get_name() << ", " << gene_expression.get_name() << ", " << clustering.get_name();
					cout.flush();
					if (bait_indices.size() < 5) {
						cout << ": Skipping: Too few genes of interest found in dataset: " << bait_indices.size() << " < 5\n";
						continue;
					}
					else {
						// Rank genes
						string name = (make_string() << job.get_name() << ".txt").str();
						replace(begin(name), end(name), ' ', '_');
						auto ranking = make_unique<Ranking>(bait_indices, clustering_, gene_correlations, name);
						cout << ": AUSR=" << setprecision(2) << fixed << ranking->get_ausr() << "\n";
						result.add(std::move(ranking));
					}
				}
				catch (MismatchException ex) {
					cout << "Info: Skipping " << clustering << " for " << gene_expression << " due to no overlap in set of genes\n";
				}
			}

			// save best result if any
			if (result.has_rankings()) {
				result.get_best_ranking().save(output_path, top_k, job, result.get_average_ausr(), output_yaml);
			}
			else {
				cout << "Warning: No output for " << job.get_name() << " due to lack of relevant gene expression data and clustering data\n";
			}
		}
	}
}

}}

