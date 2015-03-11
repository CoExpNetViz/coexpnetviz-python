// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp> // boost::shared_ptr
#include <boost/serialization/unique_ptr.hpp> // std::unique_ptr, not supported in collections though
#include <boost/serialization/variant.hpp>
#include <boost/serialization/optional.hpp>
#include <deep_blue_genome/common/util.h>

// TODO extract next few blocks to util/serialization

// Replacement for #include <boost/serialization/vector.hpp> (don't include boost's)
// Uses move instead of copying
// http://stackoverflow.com/questions/13347776/boost-serialization-of-stl-collection-of-std-unique-ptrs
namespace boost { namespace serialization {
//NOTE: do not include boost/serialization/vector.hpp
template<class Archive, class T, class Allocator>
inline void save(
    Archive & ar,
    const std::vector<T, Allocator> &t,
    const unsigned int
){
    collection_size_type count (t.size());
    ar << BOOST_SERIALIZATION_NVP(count);
    for(auto it=t.begin(), end=t.end(); it!=end; ++it)
        ar << boost::serialization::make_nvp("item", (*it));
}

template<class Archive, class T, class Allocator>
inline void load(
    Archive & ar,
    std::vector<T, Allocator> &t,
    const unsigned int
){
    collection_size_type count;
    ar >> BOOST_SERIALIZATION_NVP(count);
    t.clear();
    t.reserve(count);
    while( count-- > 0 ) {
        T item;
        ar >> boost::serialization::make_nvp("item", item);
        t.push_back(std::move(item)); // use std::move
    }
}

template<class Archive, class T, class Allocator>
inline void serialize(
    Archive & ar,
    std::vector<T, Allocator> & t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}

}} // namespace boost::serialization

// Replacement for #include <boost/serialization/unordered_map.hpp>
// Uses move instead of copying
namespace boost { namespace serialization {
//NOTE: do not include boost/serialization/vector.hpp
template<class Archive, class K, class V, class Allocator>
inline void save(
    Archive & ar,
    const std::unordered_map<K, V, Allocator> &t,
    const unsigned int
){
    collection_size_type count (t.size());
    ar << BOOST_SERIALIZATION_NVP(count);
    for(auto it=t.begin(), end=t.end(); it!=end; ++it) {
        ar << boost::serialization::make_nvp("first", it->first);
    	ar << boost::serialization::make_nvp("second", it->second);
    }
}

template<class Archive, class K, class V, class Allocator>
inline void load(
    Archive & ar,
    std::unordered_map<K, V, Allocator> &t,
    const unsigned int
){
    collection_size_type count;
    ar >> BOOST_SERIALIZATION_NVP(count);
    t.clear();
    t.reserve(count);
    while( count-- > 0 ) {
        K key;
        V value;
        ar >> boost::serialization::make_nvp("first", key);
        ar >> boost::serialization::make_nvp("second", value);
        t.emplace(std::move(key), std::move(value)); // use std::move
    }
}

template<class Archive, class T, class Allocator>
inline void serialize(
    Archive & ar,
    std::unordered_map<T, Allocator> & t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}

}} // namespace boost::serialization

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



