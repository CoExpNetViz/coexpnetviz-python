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

#pragma once

#include <boost/spirit/include/qi.hpp>

namespace DEEP_BLUE_GENOME {

/**
 * Grammar rules for tab separated files
 */
template <class Iterator>
class BasicTabGrammarRules {
public:
	BasicTabGrammarRules()
	{
		using namespace boost::spirit::qi;
		using namespace boost::fusion;

		separator = lit("\t");
		field %= as_string[lexeme[*(char_- (separator | eol))]];
		line %= field % separator;
		line_separator = +(lit("\n") | lit("\r"));

		separator.name("field separator");
		field.name("field");
		line.name("line");
	}

	boost::spirit::qi::rule<Iterator> separator; // field separator
	boost::spirit::qi::rule<Iterator, std::string()> field;
	boost::spirit::qi::rule<Iterator, std::vector<std::string>()> line; // = fields
	boost::spirit::qi::rule<Iterator> line_separator; // separates lines (also sucks up any blank lines)
};

typedef BasicTabGrammarRules<const char*> TabGrammarRules;

}
