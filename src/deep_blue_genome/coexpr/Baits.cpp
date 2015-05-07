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

		phrase_parse(begin, end, +gene, separator, genes); // TODO they return false upon failure, so we should check for that (or do they have exceptions? Because they do throw them; so it's probably alright without the check on return)
		return begin;
	});
}

const vector<std::string>& Baits::get_genes() const {
	return genes;
}

}}
