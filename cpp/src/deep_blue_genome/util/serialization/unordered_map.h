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
