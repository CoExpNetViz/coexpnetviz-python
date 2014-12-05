// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneDescriptions.h"
#include "util.h"
#include <boost/spirit/include/qi.hpp>

using namespace std;

namespace MORPHC {

GeneDescriptions::GeneDescriptions(string path)
{
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;
		using namespace boost::fusion;

		auto on_line = [this](const std::vector<std::string>& line) {
			auto gene_name = line.at(0);
			auto description = line.at(1);
			if (!descriptions.emplace(gene_name, description).second) {
				cout << "Warning: Found multiple descriptions for gene: " << gene_name << "\n";
			}
		};
		parse(begin, end, (as_string[lexeme[*(char_-(char_("\t")|eol))]] % lit("\t"))[on_line] % eol);
		// TODO use a grammar instead, check whether it degrades/improves performance
		/*auto sep = char_("\t"); // separator
		auto field = as_string[+(char_-(sep|eol))];
		parse(begin, end, ((field > sep > field)[on_mapping_item] > sep > *(char_-eol)) % eol);*/
		return begin;
	});
}

std::string GeneDescriptions::get(std::string gene) const {
	return descriptions.at(gene);
}

}
