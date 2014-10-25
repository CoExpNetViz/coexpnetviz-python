// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <unordered_set>
#include <iomanip>
#include <boost/spirit/include/qi.hpp>

#include "Clustering.h"
#include "ublas.h"
#include "util.h"
#include "Ranking.h"

// TODO fiddling with matrix orientation, would it help performance?

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

class JobGroup {
public:
	JobGroup(std::string path);

	std::string get_gene_expression();
	void add_clustering(std::string path);
	const std::vector<std::string> get_clusterings();

private:
	std::string gene_expression_path;
public:// TODO only for dbg
	std::vector<std::string> clustering_paths;
};

JobGroup::JobGroup(std::string path)
:	gene_expression_path(path)
{
}

std::string JobGroup::get_gene_expression() {
	return gene_expression_path;
}

void JobGroup::add_clustering(std::string path) {
	clustering_paths.emplace_back(path);
}

const std::vector<std::string> JobGroup::get_clusterings() {
	return clustering_paths;
}

class Application
{
public:
	void run();

private:
	void load_genes_of_interest_sets();
	void load_job_list();

private:
	std::vector<GenesOfInterest> genes_of_interest_sets;

	// job list grouped by gene_expression. Contains (gene expression, clustering) combos that need to be mined
	std::vector<JobGroup> job_list;
};

#include <fstream>

void Application::run() {
	load_genes_of_interest_sets();
	load_job_list();

	// run jobs
	for (auto& job_group : job_list) {
		GeneExpression gene_expression(job_group.get_gene_expression());

		// translate gene names to indices; and drop genes missing from the gene expression data
		std::vector<std::vector<size_type>> goi_sets; // TODO could try sets
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
		for (auto clustering_path : job_group.get_clusterings()) {
			Clustering clustering(clustering_path, gene_expression);
			for (auto& goi : goi_sets) {
				copy(goi.begin(), goi.end(), ostream_iterator<size_type>(cout, " "));
				cout << endl;

				// eliminate genes missing in clustering from goi
				std::vector<size_type> goi_(goi.size());
				auto& clustered_genes = clustering.get_genes();
				auto is_part_of_clustering = [&clustered_genes](size_type gene) {
					return clustered_genes.find(gene) != clustered_genes.end();
				};
				auto it = copy_if(goi.begin(), goi.end(), goi_.begin(), is_part_of_clustering);
				goi_.resize(std::distance(goi_.begin(), it));

				if (goi_.empty()) {
					throw runtime_error("Not implemented");// TODO return empty result with warning
				}
				else {
					// Rank genes
					Ranking ranking(goi_, clustering);
					//results.emplace_back(ranking);
				}
			}
			cout << "next clustering" << endl;
		}
	}

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
	/*for (auto& clustering : clusterings) {

	}*/

	// TODO print results
}

void Application::load_genes_of_interest_sets() {
	// Load genes of interest sets
	std::vector<string> goi_paths = {"/home/limyreth/doc/internship/data/Configs/InputTextGOI2.txt", "/home/limyreth/doc/internship/data/Configs/InputTextGOI3.txt"};
	for (string path : goi_paths) {
		genes_of_interest_sets.emplace_back(path);
		genes_of_interest_sets.back().get_genes();
	}
}

void Application::load_job_list() {
	// Load  jobs from config
	string config_path = "/home/limyreth/doc/internship/data/Configs/ConfigsBench.txt";
	std::vector<std::pair<string, string>> jobs;
	read_file(config_path, [this, &jobs](const char* begin, const char* end) {
		using namespace boost::spirit::qi;
		using namespace boost::fusion;

		auto on_job = [&jobs](const boost::fusion::vector<std::string, std::string>& job) {
			// TODO no hack
			std::string gene_expression_path = "/home/limyreth/doc/internship/data/" + at_c<0>(job);
			std::string clustering_path = "/home/limyreth/doc/internship/data/" + at_c<1>(job);
			cout << clustering_path << endl;

			// TODO canonical paths
			jobs.push_back(make_pair(gene_expression_path, clustering_path));
		};

		phrase_parse(begin, end, (as_string[lexeme[+(char_ - space)]] > as_string[lexeme[+(char_ - space)]])[on_job] % eol, blank);
		return begin;
	});

	// Group jobs by gene_expression
	sort(jobs.begin(), jobs.end());
	for (auto& job : jobs) {
		if (job_list.empty() || job_list.back().get_gene_expression() != job.first) {
			job_list.emplace_back(JobGroup(job.first));
		}
		job_list.back().add_clustering(job.second);
	}
}

// TODO perhaps not all asserts should be disabled at runtime (e.g. input checks may want to remain)
int main() {
	cout << setprecision(9);
	Application app;
	app.run();
	return 0;
}
