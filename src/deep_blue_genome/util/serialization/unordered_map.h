// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <unordered_map>
#include <boost/serialization/serialization.hpp>

// Replacement for #include <boost/serialization/unordered_map.hpp>
// Uses move instead of copying
namespace boost { namespace serialization {
//NOTE: do not include boost/serialization/unordered_map.hpp
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

template<class Archive, class K, class V, class Allocator>
inline void serialize(
    Archive & ar,
	std::unordered_map<K, V, Allocator> &t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}

}} // namespace boost::serialization
