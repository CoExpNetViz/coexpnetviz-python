// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <functional>
#include <deep_blue_genome/util/function_traits.h>

namespace DEEP_BLUE_GENOME {

/**
 * Taken from http://en.wikipedia.org/wiki/Variadic_template, see that page for usage
 *
 * Can be used to evaluate each arg with parameter unpacking magic.
 */
struct execute_variadic {
    template<typename ...T> execute_variadic(T...) {}
};

/**
 * Lambda to std::function
 */
template <class Lambda> // TODO move to util/functional.h
typename function_traits<Lambda>::function_type make_function(const Lambda& f) {
	return typename function_traits<Lambda>::function_type(f);
}

} // end namespace
