// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/util/template_magic.h>
#include <deep_blue_genome/util/printer/printer.h>

namespace DEEP_BLUE_GENOME {

/**
 * Intercalate a range of items with a delimiter.
 *
 * Like haskell intercalate.
 */
template <class Range>
Printer intercalate(const std::string& delimiter, const Range& items)
{
	return make_printer([&delimiter, &items](std::ostream& out){
		bool first = true;
		for (const auto& item : items) {
			if (!first) {
				out << delimiter;
			}
			out << item;
			first = false;
		}
	});
}


/**
 * Intercalate a variadic argument list with a delimiter.
 *
 * Like haskell intercalate.
 *
 * Args: delimiter, variadic items. Needs at leat 2 variadic items.
 */
template<class T, class... ItemsLeft>
Printer intercalate_(const std::string& delimiter, T item, const ItemsLeft&... items_left) {
	return make_printer([&delimiter, &item, &items_left...](std::ostream& out) {
		out << item;
		out << delimiter;
		execute_variadic{((out << items_left) , 1)...};
	});
}

} // end namespace
