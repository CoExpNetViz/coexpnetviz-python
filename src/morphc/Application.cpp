// Author: Tim Diels <timdiels.m@gmail.com>

#include "Application.h"
#include <iostream>
#include <iomanip>
#include <libgen.h>
#include "ublas.h"
#include "util.h"

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

namespace MORPHC {

Application::Application(int argc, char** argv)
{
	cout << setprecision(9);

	if (argc != 3) {
		cerr << "USAGE: morphc path/to/joblist.yaml path/to/output_directory top_k" << endl
				<< endl
				<< "top_k = max number of candidate genes to save in outputted rankings" << endl
				<< endl << endl;
		throw runtime_error("Invalid argument count");
	}

	string install_directory = dirname(argv[0]);
	config_path = install_directory + "/config.yaml";
	job_list_path = argv[1];
	output_path = argv[2];
	istringstream str(argv[3]);
	str >> top_k;
	// TODO look for asserts and see whether they should perhaps be needed at release as well
}

void Application::run() {
	load_config();
	load_jobs();

	for (auto& species_ : species) {
		species_.run_jobs(output_path, top_k);
	}
}

void Application::load_config() {
	YAML::Node config = YAML::LoadFile(config_path);
	string data_root = config["species_data_path"].as<string>(".");

	for (auto species_ : config["species"]) {
		species.emplace_back(data_root, species_);
	}
}

void Application::load_jobs() {
	YAML::Node job_list = YAML::LoadFile(job_list_path);
	string data_root = job_list["data_path"].as<string>(".");

	for (auto job_group : job_list["jobs"]) {
		string species_name = job_group["species_name"].as<string>();
		auto matches_name = [&species_name](const CONFIG::Species& s){
			return s.get_name() == species_name;
		};
		auto it = find_if(species.begin(), species.end(), matches_name);
		if (it == species.end()) {
			throw runtime_error("Unknown species in job list: " + species_name);
		}

		string data_root2 = prepend_path(data_root, job_group["data_path"].as<string>("."));

		for (auto goi : job_group["genes_of_interest"]) {
			it->add_job(data_root2, goi);
		}
	}
}

}

