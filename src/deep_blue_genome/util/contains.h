// Author: Tim Diels <timdiels.m@gmail.com>

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
