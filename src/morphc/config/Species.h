// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "GeneExpression.h"
#include "GenesOfInterest.h"

namespace MORPHC {
namespace CONFIG {

// A species
class Species {
public:
	Species(std::string data_root, YAML::Node species);

	void add_job(std::string data_root, YAML::Node job);
	void run_jobs();

	std::string get_name() const;

private:
	std::string name;
	std::vector<GeneExpression> gene_expressions;
	std::vector<GenesOfInterest> genes_of_interest_sets; // are treated as jobs
};

}}
