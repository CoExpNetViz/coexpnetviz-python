// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <morphc/util.h>

using namespace std;

namespace MORPHC {
namespace CONFIG {

GeneExpression::GeneExpression(string data_root, YAML::Node node)
:	name(node["name"].as<string>()), gene_expression_path(data_root + "/" + node["path"].as<string>())
{
	for (auto path : node["clusterings"]) {
		clustering_paths.emplace_back(prepend_path(data_root, path.as<string>()));
	}
}

std::string GeneExpression::get_path() {
	return gene_expression_path;
}

const std::vector<std::string> GeneExpression::get_clusterings() {
	return clustering_paths;
}

}}
