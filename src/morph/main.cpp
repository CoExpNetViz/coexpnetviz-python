// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>

#include <string>
#include <vector>
#include <map>
#include <utility>

#include "Clustering.h"
#include "ublas.h"
#include "util.h"

// TODO fiddling with matrix orientation, would it help performance?

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

class JobGroup {
public:
	JobGroup(std::string path);

	std::string get_gene_expression();
	void add_clustering(std::string path);

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

class Application
{
public:
	void run();

private:
	void load_genes_of_interest_sets();
	void load_job_list();

private:
	std::map<std::string, GeneExpression> gene_expression_sets; // source path -> GeneExpression
	std::vector<Clustering> clusterings;
	std::vector<GenesOfInterest> genes_of_interest_sets;
	std::vector<string> all_genes_of_interest;

	// job list grouped by gene_expression. Contains (gene expression, clustering) combos that need to be mined
	std::vector<JobGroup> job_list;
};

#include <fstream>

void Application::run() {
	load_genes_of_interest_sets();
	load_job_list();

	for (auto& job : job_list) {
		cout << job.get_gene_expression() << ":" << endl;
		copy(job.clustering_paths.begin(), job.clustering_paths.end(), ostream_iterator<string>(cout, "\n"));
		cout << endl;
		cout << endl;
	}
	// load first job TODO for all jobs
	/*auto it = gene_expression_sets.find(gene_expression_path);
	if (it == gene_expression_sets.end()) {
		it = gene_expression_sets.emplace(gene_expression_path, GeneExpression(gene_expression_path, all_genes_of_interest)).first;
	}

	clusterings.emplace_back(Clustering(clustering_path, it->second));*/

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

void Application::load_genes_of_interest_sets() {
	// Load genes of interest sets
	std::vector<string> goi_paths = {"../data/Configs/InputTextGOI2.txt", "../data/Configs/InputTextGOI3.txt"};
	for (string path : goi_paths) {
		genes_of_interest_sets.emplace_back(path);
		auto& goi = genes_of_interest_sets.back().get_genes();
		all_genes_of_interest.insert(all_genes_of_interest.end(), goi.begin(), goi.end());
	}
	sort(all_genes_of_interest.begin(), all_genes_of_interest.end());
	all_genes_of_interest.erase(unique(all_genes_of_interest.begin(), all_genes_of_interest.end()), all_genes_of_interest.end());
}

void Application::load_job_list() {
	// Load  jobs from config
	string config_path = "../data/Configs/ConfigsBench.txt";
	std::vector<std::pair<string, string>> jobs;
	read_file(config_path, [this, &jobs](ifstream& in) {
		while (in.good()) {
			string gene_expression_path;
			string clustering_path;
			in >> gene_expression_path;
			if (in.fail()) {
				break;
			}
			in >> clustering_path;
			if (in.fail()) {
				throw runtime_error("Incomplete job description line");
			}

			// TODO no hack
			gene_expression_path = "../data/" + gene_expression_path;
			clustering_path = "../data/" + clustering_path;

			// TODO canonical paths
			jobs.push_back(make_pair(gene_expression_path, clustering_path));
		}
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

// TODO loading everything at start of prog is probably too wasteful of memory, load each ~config-line one by one
int main() {
	Application app;
	app.run();
	return 0;
}
