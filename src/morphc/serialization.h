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

#pragma once

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

namespace MORPHC {

/**
 * Load object saved with save_to_binary
 */
template <class T>
void load_from_binary(std::string path, T& object) {
	using namespace std;
	using namespace boost::iostreams;
	stream_buffer<file_source> buffer(path, ios::binary); // Note: this turned out to be strangely slightly faster than mapped_file
	boost::archive::binary_iarchive ar(buffer);
	ar >> object;
}

template <class T>
void save_to_binary(std::string path, const T& object) {
	using namespace std;
	using namespace boost::iostreams;
	stream_buffer<file_sink> out(path, ios::binary);
	boost::archive::binary_oarchive ar(out);
	ar << object;
}

/**
 * Loads bin if possible, else plain
 *
 * Very specialised function, limited reusability
 */
template <class T>
void load_bin_or_plain(std::string path, T& object) {
	using namespace std;
	auto path_bin = path + ".bin";
	if (boost::filesystem::exists(path_bin)) {
		// load from binary file
		cout << "loading bin" << endl;
		load_from_binary(path_bin, object);
	}
	else {
		cout << "plain+save" << endl;
		object.load_plain(path);

		// save bin so we can load file more quickly next time (reading formatted plain text takes longer than reading binary)
		save_to_binary(path_bin, object);
	}
}


}
