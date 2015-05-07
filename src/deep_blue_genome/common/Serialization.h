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

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/optional.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/util/serialization/unique_ptr.h> // std::unique_ptr, not supported in all collections, but it's fairly easy to add an implementation by copy pasting in util/serialization
#include <deep_blue_genome/util/serialization/unordered_set.h>
#include <deep_blue_genome/util/serialization/forward_list.h>
#include <deep_blue_genome/util/serialization/list.h>
#include <deep_blue_genome/util/serialization/unordered_map.h>
#include <deep_blue_genome/util/serialization/vector.h>
#include <deep_blue_genome/util/serialization/flat_set.h>
#include <deep_blue_genome/util/serialization/flat_map.h>

namespace DEEP_BLUE_GENOME {

class Serialization
{
public:
	template <class T>
	static void load_from_binary(std::string path, T& object) {
		using namespace std;
		using namespace boost::iostreams;
		stream_buffer<file_source> buffer(path, ios::binary); // Note: this turned out to be strangely slightly faster than mapped_file
		boost::archive::binary_iarchive ar(buffer);
		ar >> object;
	}

	template <class T>
	static void save_to_binary(std::string path, const T& object) {
		using namespace std;
		using namespace boost::iostreams;
		using namespace boost::filesystem;
		create_directories(boost::filesystem::path(path).remove_filename());
		stream_buffer<file_sink> out(path, ios::binary);
		boost::archive::binary_oarchive ar(out);
		try {
			ar << object;
		}
		catch (const exception& e) {
			throw runtime_error("Error while writing to " + path + ": " + exception_what(e));
		}
	}
};


} // end namespace



