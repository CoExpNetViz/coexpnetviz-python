// Author: Tim Diels <timdiels.m@gmail.com>

#include "Application.h"
#include <iostream>
#include "ublas.h"
#include "util.h"

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

namespace MORPHC {

Application::Application(std::string job_list_path)
:	job_list_path(job_list_path)
{
}

void Application::run() {
	load_config();
	load_jobs();

	for (auto& species_ : species) {
		species_.run_jobs();
	}
}

void Application::load_config() {
	string config_path = "config.yaml"; // TODO should be relative to installation dir (basedir(argv[0])/config.yaml should do the trick)
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

