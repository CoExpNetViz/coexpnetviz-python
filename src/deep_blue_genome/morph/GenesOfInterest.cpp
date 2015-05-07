/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

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

		phrase_parse(begin, end, +gene, separator, genes);
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

void GenesOfInterest::canonicalise(const Canonicaliser& mapping) {
	vector<string> new_genes;
	for (auto& gene : genes) {
		auto genes = mapping.get(gene);
		new_genes.insert(new_genes.end(), genes.begin(), genes.end());
	}
	genes.swap(new_genes);

	// Remove duplicates
	sort(genes.begin(), genes.end());
	auto unique_end = unique(genes.begin(), genes.end());
	genes.erase(unique_end, genes.end());
}

}}
