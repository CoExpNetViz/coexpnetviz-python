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

namespace DEEP_BLUE_GENOME {

/**
 * Grammar rules for tab separated files
 */
template <class Iterator>
class BasicTabGrammarRules {
public:
	BasicTabGrammarRules(bool ignoreEmptyFields)
	{
		using namespace boost::spirit::qi;
		using namespace boost::fusion;

		field_separator = lit("\t");
		field %= as_string[lexeme[*(char_- (field_separator | eol))]];
		if (ignoreEmptyFields) {
			line %= field % +field_separator;
		}
		else {
			line %= field % field_separator;
		}
		line_separator = +(lit("\n") | lit("\r"));

		field_separator.name("field separator");
		field.name("field");
		line.name("line");
	}

	boost::spirit::qi::rule<Iterator> field_separator; // field separator
	boost::spirit::qi::rule<Iterator, std::string()> field;
	boost::spirit::qi::rule<Iterator, std::vector<std::string>()> line; // = fields
	boost::spirit::qi::rule<Iterator> line_separator; // separates lines (also sucks up any blank lines)
};

typedef BasicTabGrammarRules<const char*> TabGrammarRules;

}
