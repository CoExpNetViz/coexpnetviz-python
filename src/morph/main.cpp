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
	//results.reserve(clusterings.size());

	for (auto& clustering : clusterings) {
		for (auto& genes_of_interest : genes_of_interest_sets) {
			Ranking ranking(genes_of_interest, clustering);
			//results.emplace_back(ranking);
		}
	}*/

	// TODO print results
}

void Application::load() {
	// Load config: lists clustering files and their associated gene expression files
	string config_path = "../data/Configs/ConfigsBench.txt";
	try {
		ifstream in(config_path);
		in.exceptions(ios::badbit);
		while (in.good()) {
			string gene_expression_path;
			string clustering_path;
			in >> gene_expression_path >> clustering_path;
			if (in.fail()) {
				throw runtime_error("Syntax error");
			}

			/*auto it = gene_expression_sets.find(gene_expression_path);
			if (it == gene_expression_sets.end()) {
				it = gene_expression_sets.emplace(gene_expression_path, GeneExpression(gene_expression_path)).first;
			}

			clusterings.emplace_back(Clustering(clustering_path, it->second));*/
		}
	}
	catch (exception& e) {
		throw runtime_error((make_string() << "Error while reading config file (" << config_path << "): " << e.what()).str());
	}

	// Load gene of interest sets
	// TODO genes_of_interest_sets
	std::vector<string> goi_paths = {"../data/Configs/InputTextGOI2.txt", "../data/Configs/InputTextGOI3.txt"};
	for (string path : goi_paths) {
		try {
			ifstream in(path);
			in.exceptions(ios::badbit);
			while (in.good()) {
				string gene_name;
				in >> gene_name;
				if (in.fail()) {
					break;
				}
				//cout << gene_name << ",";
			}
		}
		catch (exception& e) {
			throw runtime_error((make_string() << "Error while reading genes-of-interest file (" << path << "): " << e.what()).str());
		}
		cout << endl;
	}
}

int main() {
	Application app;
	app.run();
	return 0;
}
