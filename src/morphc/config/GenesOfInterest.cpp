// Author: Tim Diels <timdiels.m@gmail.com>

#include "GenesOfInterest.h"
#include <morphc/util.h>
#include <boost/spirit/include/qi.hpp>

using namespace std;

namespace MORPHC {
namespace CONFIG {

// TODO only add data_root to relative paths
GenesOfInterest::GenesOfInterest(string data_root, YAML::Node node)
:	name(node["name"].as<string>())
{
	// Load
	auto genes_ = node["genes"];
	if (genes_) {
		for (auto gene : genes_) {
			genes.emplace_back(gene.as<string>());
		}
	}
	else {
		read_file(prepend_path(data_root, node["path"].as<string>()), [this](const char* begin, const char* end) {
			using namespace boost::spirit::qi;
			phrase_parse(begin, end, +as_string[lexeme[+(char_-space)]], space, genes);
			return begin;
		});
	}
}
const vector<std::string>& GenesOfInterest::get_genes() const {
	return genes;
}

std::string GenesOfInterest::get_name() const {
	return name;
}

}}
