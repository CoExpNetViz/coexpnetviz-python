// Author: Tim Diels <timdiels.m@gmail.com>

#include "GenesOfInterest.h"
#include <morphc/util.h>
#include <boost/spirit/include/qi.hpp>

using namespace std;

namespace MORPHC {

GenesOfInterest::GenesOfInterest(string data_root, const YAML::Node& node)
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
			typedef const char* Iterator;

			rule<Iterator> separator = space | lit(",");
			rule<Iterator, std::string()> gene;

			gene %= as_string[lexeme[+(char_-separator)]];

			separator.name("gene separator");
			gene.name("gene");

			parse(begin, end, gene % separator, genes);
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

void GenesOfInterest::apply_mapping(const GeneMapping& mapping) {
	for (auto& gene : genes) {
		if (mapping.has(gene)) {
			gene = mapping.get(gene);
		}
	}
}

}
