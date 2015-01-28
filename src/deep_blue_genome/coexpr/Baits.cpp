// Author: Tim Diels <timdiels.m@gmail.com>

#include "Baits.h"
#include <boost/spirit/include/qi.hpp>
#include <deep_blue_genome/common/util.h>

using namespace std;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

Baits::Baits(string path)
{
	// Load
	read_file(path, [this](const char* begin, const char* end) {
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

	for (auto& gene : genes) {
		to_lower(gene);
	}
}

const vector<std::string>& Baits::get_genes() const {
	return genes;
}

}}
