/*
 * Copyright (C) 2014 by Tim Diels
 *
 * This file is part of binreverse.
 *
 * binreverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * binreverse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with binreverse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"
#include <boost/iostreams/device/mapped_file.hpp>

using namespace std;

namespace MORPHC {

void read_file(std::string path, std::function<const char* (const char*, const char*)> reader) {
	try {
		boost::iostreams::mapped_file mmap(path, boost::iostreams::mapped_file::readonly);
		auto begin = mmap.const_data();
		auto end = begin + mmap.size();

		// trim last newline if any
		if (begin != end && *(end-1) == '\n') {
			end--;
			if (begin != end && *(end-1) == '\r')
				end--;
		}

		auto current = reader(begin, end);

		if (current != end) {
			ostringstream out;
			out << "Trailing characters at end of file: '";
			copy(current, end, ostream_iterator<char>(out));
			out << "'";
			throw runtime_error(out.str());
		}
	}
	catch (const exception& e) {
		throw runtime_error((make_string() << "Error while reading '" << path << "': " << e.what()).str());
	}
}

string prepend_path(string prefix, string path) {
	if (path.at(0) == '/')
		return path;
	else
		return prefix + "/" + path;
}

void ensure(bool condition, std::string error_message) {
	if (!condition)
		throw std::runtime_error(error_message);
}

}
