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

using namespace std;

void read_file(std::string path, std::function<void(ifstream&)> reader) {
	try {
		ifstream in(path);
		in.exceptions(ios::badbit);
		if (!in.good()) {
			throw runtime_error("Failed to open file");
		}
		reader(in);
	}
	catch (exception& e) {
		throw runtime_error((make_string() << "Error while reading file (" << path << "): " << e.what()).str());
	}
}
