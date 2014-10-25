// Author: Tim Diels <timdiels.m@gmail.com>

#include "GenesOfInterest.h"
#include "util.h"
#include <boost/spirit/include/qi.hpp>

using namespace std;

GenesOfInterest::GenesOfInterest(std::string path)
:	name(path)
{
	// Load
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;
		phrase_parse(begin, end, +as_string[lexeme[+(char_-space)]], space, genes);
		return begin;
	});
}
const vector<std::string>& GenesOfInterest::get_genes() {
	return genes;
}
