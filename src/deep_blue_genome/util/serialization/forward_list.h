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
