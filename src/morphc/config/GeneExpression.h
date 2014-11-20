// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "Clustering.h"

namespace MORPHC {
namespace CONFIG {

// A species
class GeneExpression {
public:
	GeneExpression(std::string data_root, YAML::Node expression_matrix);

	std::string get_name() const;
	std::string get_path() const;
	const std::vector<Clustering>& get_clusterings() const;

private:
	std::string name;
	std::string gene_expression_path;
	std::vector<Clustering> clusterings;
};

}}
