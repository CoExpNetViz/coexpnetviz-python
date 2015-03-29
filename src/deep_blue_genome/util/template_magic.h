// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

namespace DEEP_BLUE_GENOME {

/**
 * Taken from http://en.wikipedia.org/wiki/Variadic_template, see that page for usage
 *
 * Can be used to evaluate each arg with parameter unpacking magic.
 */
struct execute_variadic {
    template<typename ...T> execute_variadic(T...) {}
};

} // end namespace
