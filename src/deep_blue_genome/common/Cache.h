// Author: Tim Diels <timdiels.m@gmail.com>

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
#include <deep_blue_genome/common/util.h>

namespace MORPHC {

/**
 * Loading/saving of files
 */
class Cache {
public:
	Cache(std::string path)
	:	cache_path(path)
	{
	}

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
		try {
			ar << object;
		}
		catch (const exception& e) {
			throw runtime_error("Error while writing to " + path + ": " + exception_what(e));
		}
	}

	template <class T>
	void load_bin_or_plain(std::string path, T& object) {
		load_bin_or_plain(path, path + ".bin", object);
	}

	/**
	 * Loads bin if possible, else plain
	 *
	 * Very specialised function, limited reusability
	 */
	template <class T>
	void load_bin_or_plain(std::string path, std::string bin_name, T& object) {
		using namespace std;

		std::replace(bin_name.begin(), bin_name.end(), '/', '@');
		std::string bin_path = cache_path + "/" + bin_name;

		if (boost::filesystem::exists(bin_path)) {
			// load from binary file
			load_from_binary(bin_path, object);
		}
		else {
			object.load_plain(path);

			// save bin so we can load file more quickly next time (reading formatted plain text takes longer than reading binary)
			save_to_binary(bin_path, object);
		}
	}

private:
	std::string cache_path;  // path of cache
};


}
