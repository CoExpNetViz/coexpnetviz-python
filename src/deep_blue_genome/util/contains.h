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

#include <iostream>
#include <algorithm>
#include <type_traits>
#include <deep_blue_genome/util/template/has_member.h>

namespace DEEP_BLUE_GENOME {

namespace impl {

	template <typename Value>
	struct check_has_find {
		template <typename Container, typename Container::const_iterator (Container::*)(const Value&) const = &Container::find>
		struct get {};
	};

	template <typename Container, typename Value>
	struct has_find : has_member<Container, check_has_find<Value>>
	{
	};

}

template<typename Container, typename T>
typename std::enable_if<!impl::has_find<Container, T>::value, bool>::type contains(const Container& container, const T& value) {
    return std::find(container.begin(), container.end(), value) != container.end();
}

template<typename Container, typename T>
typename std::enable_if<impl::has_find<Container, T>::value, bool>::type contains(const Container& container, const T& value) {
    return container.find(value) != container.end();
}

} // end namespace
