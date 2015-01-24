// Author: Tim Diels <timdiels.m@gmail.com>

#include "GenesOfInterest.h"
#include <boost/spirit/include/qi.hpp>
#include <deep_blue_genome/common/util.h>

using namespace std;

namespace DEEP_BLUE_GENOME {
namespace MORPH {

GenesOfInterest::GenesOfInterest(string name, string path, const boost::regex& gene_pattern)
:	name(name)
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
		ensure(regex_match(gene, gene_pattern),
				(make_string() <<"Invalid gene name: " << gene).str(),
				ErrorType::INVALID_GOI_GENE);
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
}

}}
