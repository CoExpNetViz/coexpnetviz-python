// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/util/function_traits.h>

namespace DEEP_BLUE_GENOME {

/**
 * Lambda to std::function
 */
template <class Lambda> // TODO move to util/functional.h
typename function_traits<Lambda>::function_type make_function(const Lambda& f) {
	return typename function_traits<Lambda>::function_type(f);
}

} // end namespace
