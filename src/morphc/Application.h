// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "config/Species.h"

namespace MORPHC {

class Application
{
public:
	Application(std::string config_path, std::string job_list_path, std::string output_path);
	void run();

private:
	void load_config();
	void load_jobs();

private:
	std::vector<CONFIG::Species> species; // list of species that need to be mined
	std::string config_path;
	std::string job_list_path;
	std::string output_path;
};

}
