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

#include "GeneParserRule.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneParserRule::GeneParserRule()
{
}

GeneParserRule::GeneParserRule(const std::string& match, const std::string& replace)
:	replace_format(replace)
{
	set_matcher(match);
}

bool GeneParserRule::try_parse(std::string& name, NullableSpliceVariantId& splice_variant_id) {
	match_results<string::const_iterator> results;
	if (regex_match(name, results, matcher_re)) {
		// get splice variant
		splice_variant_id = boost::none;
		string variant_str = results[results.size()-1]; // gets the last occurrence of the last subgroup
		if (!variant_str.empty()) {
			istringstream str(variant_str);
			SpliceVariantId id;
			str >> id;
			ensure(!str.fail(),
					(make_string() << "Error parsing gene variant '" << name << "': '" << variant_str << "' is not a splice variant id").str()
			);
			splice_variant_id = id;
		}

		// get gene name
		name = results.format(replace_format);

		return true;
	}
	return false;
}

void GeneParserRule::set_matcher(const std::string& match) { // TODO if a user provides a greedy .* match, then our numbers won't be matched in it. Must document or fix this
	matcher = match + "([.]([0-9]+))*";
 	matcher_re = boost::regex(matcher, boost::regex::perl | boost::regex::icase);
}

} // end namespace
