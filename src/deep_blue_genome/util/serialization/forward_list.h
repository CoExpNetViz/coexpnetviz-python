// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <forward_list>
#include <boost/serialization/serialization.hpp>

// Replacement for #include <boost/serialization/forward_list.hpp>
// Uses move instead of copying
namespace boost { namespace serialization {
//NOTE: do not include boost/serialization/forward_list.hpp
template<class Archive, class T, class Allocator>
inline void save(
    Archive& ar,
    const std::forward_list<T, Allocator>& t,
    const unsigned int
){
    collection_size_type count (std::distance(t.begin(), t.end()));
    ar << BOOST_SERIALIZATION_NVP(count);
    for(auto& item : t) {
        ar << boost::serialization::make_nvp("item", item);
    }
}

template<class Archive, class T, class Allocator>
inline void load(
    Archive& ar,
	std::forward_list<T, Allocator>& t,
    const unsigned int
){
    collection_size_type count;
    ar >> BOOST_SERIALIZATION_NVP(count);
    t.clear();
    while( count-- > 0 ) {
        T item;
        ar >> boost::serialization::make_nvp("item", item);
        t.push_front(std::move(item));
    }
    t.reverse();
}

template<class Archive, class T, class Allocator>
inline void serialize(
    Archive& ar,
	std::forward_list<T, Allocator>& t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}

}} // namespace boost::serialization
