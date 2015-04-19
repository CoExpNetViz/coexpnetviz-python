#pragma once

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//  See http://www.boost.org for updates, documentation, and revision history.

// Copyright 2015 Tim Diels
// Modified to use unique_ptr<T, D> instead of just <T>
// Don't include boost/serialization/unique_ptr.hpp when using this

#include <memory>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/nvp.hpp>

// Replacement for #include <boost/serialization/unique_ptr.hpp>
// Uses move instead of copying
namespace boost { namespace serialization {
//NOTE: do not include boost/serialization/unique_ptr.hpp

/////////////////
// unique_ptr

template<class Archive, class T, class D>
inline void save(
    Archive & ar,
    const std::unique_ptr< T, D > &t,
    const unsigned int file_version
){
    // only the raw pointer has to be saved
    // the ref count is rebuilt automatically on load
    const T * const tx = t.get();
    ar << boost::serialization::make_nvp("pointer", tx);
    ar << boost::serialization::make_nvp("deleter", t.get_deleter());
}

template<class Archive, class T, class D>
inline void load(
    Archive & ar,
    std::unique_ptr< T, D > &t,
    const unsigned int file_version
){
    T *tx;
    D deleter;
    ar >> boost::serialization::make_nvp("pointer", tx);
    ar >> boost::serialization::make_nvp("deleter", deleter);
    // note that the reset automagically maintains the reference count
    t = std::unique_ptr<T,D>(tx, std::move(deleter));
}

template<class Archive, class T, class D>
inline void serialize(
    Archive & ar,
    std::unique_ptr< T, D > &t,
    const unsigned int file_version
){
    boost::serialization::split_free(ar, t, file_version);
}

///////////////////////
// std::default_delete

template<class Archive, class T>
inline void serialize(
    Archive & ar,
    std::default_delete<T> &t,
    const unsigned int file_version
){
    // no serialization needed
}

}} // namespace boost::serialization
