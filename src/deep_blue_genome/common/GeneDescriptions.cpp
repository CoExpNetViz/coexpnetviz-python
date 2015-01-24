// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneDescriptions.h"
#include <cctype>
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/TabGrammarRules.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

GeneDescriptions::GeneDescriptions(string path)
{
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto on_line = [this](const std::vector<std::string>& line) {
			auto gene_name = line.at(0);
			to_lower(gene_name);
			auto description = line.at(1);
			if (!mapping.emplace(gene_name, description).second) {
				cout << "Warning: Found multiple mappings for: " << gene_name << "\n";
			}
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

std::string GeneDescriptions::get(std::string gene) const {
	assert(all_of(gene.begin(), gene.end(), [](int c){ return islower(c); }));
	auto it = mapping.find(gene);
	if (it == mapping.end()) {
		//cout << "Warning: description not found for gene: " << gene << "\n";
		return "";
	}
	else {
		return it->second;
	}
}

}
