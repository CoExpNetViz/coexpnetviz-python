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

#include <boost/container/set.hpp>
#include <boost/serialization/serialization.hpp>

/**
 * Serialization of boost container `flat_set`s
 *
 * Based on http://stackoverflow.com/questions/13347776/boost-serialization-of-stl-collection-of-std-unique-ptrs
 */

namespace boost { namespace serialization {

//NOTE: do not include boost/serialization/vector.hpp
template<class Archive, class Key, class Compare, class Allocator>
inline void save(
    Archive & ar,
    const boost::container::flat_set<Key, Compare, Allocator> &t,
    const unsigned int
){
    collection_size_type count (t.size());
    ar << BOOST_SERIALIZATION_NVP(count);
    for(auto it=t.begin(), end=t.end(); it!=end; ++it)
        ar << boost::serialization::make_nvp("item", (*it));
}

template<class Archive, class Key, class Compare, class Allocator>
inline void load(
    Archive & ar,
    boost::container::flat_set<Key, Compare, Allocator> &t,
    const unsigned int
){
    collection_size_type count;
    ar >> BOOST_SERIALIZATION_NVP(count);
    t.clear();
    t.reserve(count);
    while( count-- > 0 ) {
        Key item;
        ar >> boost::serialization::make_nvp("item", item);
        t.emplace(std::move(item));
    }
}

template<class Archive, class Key, class Compare, class Allocator>
inline void serialize(
    Archive & ar,
    boost::container::flat_set<Key, Compare, Allocator> &t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}

}} // namespace boost::serialization
