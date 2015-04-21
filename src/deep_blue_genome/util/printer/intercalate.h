// Author: Tim Diels <timdiels.m@gmail.com>

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
