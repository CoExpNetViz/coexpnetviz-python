// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <boost/serialization/serialization.hpp>

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
