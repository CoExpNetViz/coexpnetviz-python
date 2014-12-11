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
	vector<string> new_genes;
	for (auto& gene : genes) {
		if (mapping.has(gene)) {
			auto genes = mapping.get(gene);
			new_genes.insert(new_genes.end(), genes.begin(), genes.end());
		}
		else {
			new_genes.emplace_back(gene);
		}
	}
	genes.swap(new_genes);

	// Remove duplicates
	sort(genes.begin(), genes.end());
	auto unique_end = unique(genes.begin(), genes.end());
	genes.erase(unique_end, genes.end());

	// Debug
	copy(genes.begin(), genes.end(), ostream_iterator<std::string>(cout, " "));
	cout << endl;
}

}
