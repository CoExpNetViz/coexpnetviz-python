// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <sstream>

namespace DEEP_BLUE_GENOME {

/**
 * Prints to stream using provided function
 *
 * @param printer_func Function with signature void(ostream&)
 */
template <class F>
std::string make_printer(F&& printer_func) {
	std::ostringstream out;
	printer_func(out);
	return out.str();
}

} // end namespace
