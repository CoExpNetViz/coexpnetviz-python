// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneDescriptions.h"
#include "util.h"
#include <morphc/TabGrammarRules.h>

using namespace std;

namespace MORPHC {

GeneDescriptions::GeneDescriptions(string path)
{
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto on_line = [this](const std::vector<std::string>& line) {
			auto gene_name = line.at(0);
			auto description = line.at(1);
			if (!descriptions.emplace(gene_name, description).second) {
				cout << "Warning: Found multiple descriptions for gene: " << gene_name << "\n";
			}
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
}

std::string GeneDescriptions::get(std::string gene) const {
	auto it = descriptions.find(gene);
	if (it == descriptions.end()) {
		//cout << "Warning: description not found for gene: " << gene << "\n";
		return "";
	}
	else {
		return it->second;
	}
}

}
