// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <exception>
#include <sstream>
#include <map>
#include <functional>
#include <boost/spirit/include/qi.hpp>
#include <deep_blue_genome/common/ErrorType.h>

namespace DEEP_BLUE_GENOME {

class NotFoundException : public std::runtime_error {
public:
	NotFoundException(const std::string& kind)
	:	runtime_error(kind)
	{
	}
};

// This class taken from: http://stackoverflow.com/a/25351759/1031434
// Contributed by Jason R
class make_string
{
public:
	make_string()
	{
	}

    template <typename T>
    explicit make_string(T && rhs)
    {
        oss << rhs;
    }

    template <typename T>
    make_string &operator<<(T && rhs)
    {
        oss << rhs;
        return *this;
    }

    operator std::string() const
    {
        return oss.str();
    }

    std::string str() const
    {
        return oss.str();
    }

private:
    std::ostringstream oss;
};

/**
 * Find greatest key smaller than given key
 */
template <class K, class V>
typename std::map<K,V>::const_iterator infimum(const std::map<K, V>& map_, const K& value) {
	auto it = map_.upper_bound(value);
	assert (it != map_.begin());
	it--;
	return it;
}

/**
 * Open plain text file for efficient reading, calls reader when it's open with begin and exclusive end of file data
 *
 * reader returns the last position it read at. If that's not equal to end, then a trailing-chars exception is thrown
 */
void read_file(std::string path, std::function<const char* (const char* begin, const char* end)> reader);

template<class Container, class T>
bool contains(const Container& container, const T& value) {
	return std::find(container.begin(), container.end(), value) != container.end();
}

/**
 * Prepend prefix to path if path is relative path
 */
std::string prepend_path(std::string prefix, std::string path);

/**
 * Like assert, but is included in release versions as well
 */
void ensure(bool condition, std::string error_message, ErrorType);

/**
 * Format exception to string
 *
 * Named after exception.what()
 */
std::string exception_what(const std::exception& e);

void to_lower(std::string& data);

/**
 * Call graceful_main, exit gracefully after it returns/throws with pretty error output
 */
void graceful_main(std::function<void()> fragile_main);


template<class T>
class Iterable
{
public:
    Iterable(T begin, T end)
    :	_begin(begin), _end(end)
    {
    }

    T begin() {
        return _begin;
    }

    T end() {
        return _end;
    }

private:
    T _begin;
    T _end;
};

template<class T>
Iterable<T> make_iterable(T t, T u)
{
    return Iterable<T>(t, u);
}

/**
 * Encode string in such a way that it's a valid file name.
 *
 * This mapping has no collisions. (i.e. f(x) == f(y) iff x=y).
 * Its encoding is similar url encoding (illegal characters replaced by %XX)
 */
std::string to_file_name(const std::string&);

/**
 * Code from boost
 * Reciprocal of the golden ratio helps spread entropy
 *     and handles duplicates.
 * See Mike Seymour in magic-numbers-in-boosthash-combine:
 *     http://stackoverflow.com/questions/4948780
 *
 * @example hash of tuple (x,y): seed=0; hash_combine(seed, x); hash_combine(seed, y); return seed;
 */
template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
	seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

} // end namespace

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}



