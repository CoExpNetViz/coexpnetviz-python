#pragma once

/**
 * Function traits as described here: http://stackoverflow.com/a/7943765/1031434
 */

namespace DEEP_BLUE_GENOME {

/**
 * Tip: you can get types of all args with: traits::arg<int_range<traits::arity>{}>::type...
 */
template <typename T>
struct function_traits
    : public function_traits<decltype(&T::operator())>
{};
// For generic types, directly use the result of the signature of its 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    enum { arity = sizeof...(Args) };
    // arity is the number of arguments.

    typedef ReturnType return_type;

    template <size_t i>
    struct arg
    {
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
        // the i-th argument is equivalent to the i-th tuple element of a tuple
        // composed of those arguments.
    };

    typedef std::function<ReturnType(Args...)> function_type;
};

} // end namespace
