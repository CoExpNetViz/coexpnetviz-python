// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <unordered_set>
#include <boost/serialization/serialization.hpp>

// Replacement for #include <boost/serialization/unordered_set.hpp>
// Uses move instead of copying
namespace boost { namespace serialization {
//NOTE: do not include boost/serialization/unordered_set.hpp
template<class Archive, class Key, class Hash, class KeyEqual, class Allocator>
inline void save(
    Archive & ar,
    const std::unordered_set<Key, Hash, KeyEqual, Allocator> &t,
    const unsigned int
){
    collection_size_type count (t.size());
    ar << BOOST_SERIALIZATION_NVP(count);
    for(auto& item : t) {
        ar << boost::serialization::make_nvp("item", item);
    }
}

template<class Archive, class Key, class Hash, class KeyEqual, class Allocator>
inline void load(
    Archive & ar,
    std::unordered_set<Key, Hash, KeyEqual, Allocator> &t,
    const unsigned int
){
    collection_size_type count;
    ar >> BOOST_SERIALIZATION_NVP(count);
    t.clear();
    t.reserve(count);
    while( count-- > 0 ) {
        Key key;
        ar >> boost::serialization::make_nvp("item", key);
        t.emplace(std::move(key));
    }
}

template<class Archive, class Key, class Hash, class KeyEqual, class Allocator>
inline void serialize(
    Archive & ar,
	std::unordered_set<Key, Hash, KeyEqual, Allocator> &t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}

}} // namespace boost::serialization
