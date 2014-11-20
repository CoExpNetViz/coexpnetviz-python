// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <morphc/util.h>

using namespace std;

namespace MORPHC {
namespace CONFIG {

GeneExpression::GeneExpression(string data_root, YAML::Node node)
:	name(node["name"].as<string>()), gene_expression_path(data_root + "/" + node["path"].as<string>())
{
	for (auto clustering : node["clusterings"]) {
		clusterings.emplace_back(clustering["name"].as<string>(), prepend_path(data_root, clustering["path"].as<string>()));
	}
}

std::string GeneExpression::get_path() const {
	return gene_expression_path;
}

const std::vector<Clustering>& GeneExpression::get_clusterings() const {
	return clusterings;
}

std::string GeneExpression::get_name() const {
	return name;
}

}}
