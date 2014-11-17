// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace MORPHC {
namespace CONFIG {


// A species
class GeneExpression {
public:
	GeneExpression(std::string data_root, YAML::Node expression_matrix);

	std::string get_path();
	const std::vector<std::string> get_clusterings();

private:
	std::string name;
	std::string gene_expression_path;
	std::vector<std::string> clustering_paths;
};

}}
