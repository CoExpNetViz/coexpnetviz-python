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

#include <sstream>
#include <deep_blue_genome/util/functional.h>

namespace DEEP_BLUE_GENOME {
// TODO can update to what we have in cpp_future (minus the && madness as we don't need that)

/**
 * Private things
 */
namespace impl {
	template <class Delimiter>
	void intercalate_(std::ostream& out, const Delimiter& delimiter) {
	}

	template <class Delimiter, class Item, class... Items>
	void intercalate_(std::ostream& out, const Delimiter& delimiter, const Item& item, const Items&... items)
	{
		out << delimiter << item;
		intercalate_(out, delimiter, items...);
	}
}

/**
 * Intercalate a range of items with a delimiter.
 *
 * Like haskell intercalate.
 *
 * Args: delimiter, range of items
 */
template <class Delimiter, class Range>
std::string intercalate(const Delimiter& delimiter, const Range& items)
{
	std::ostringstream out;

	bool first = true;
	for (auto&& item : items) {
		if (!first) {
			out << delimiter;
		}
		out << item;
		first = false;
	}

	return out.str();
}

/**
 * Intercalate a variadic argument list with a delimiter.
 *
 * Like haskell intercalate.
 *
 * Args: delimiter, item1, item2, ..., itemN.
 */
template <class Delimiter, class Item, class... Items>
std::string intercalate_(const Delimiter& delimiter, const Item& first_item, const Items&... items) {
	std::ostringstream out;
	out << first_item;
	impl::intercalate_(out, delimiter, items...);
	return out.str();
}

} // end namespace
