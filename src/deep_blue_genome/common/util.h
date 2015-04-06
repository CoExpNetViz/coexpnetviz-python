// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <exception>
#include <map>
#include <utility>
#include <functional>
#include <boost/spirit/include/qi.hpp>
#include <deep_blue_genome/common/ErrorType.h>
#include <deep_blue_genome/util/make_string.h>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>

#define GCC_VERSION (__GNUC__ * 10000 \
					   + __GNUC_MINOR__ * 100 \
					   + __GNUC_PATCHLEVEL__)

// TODO got some priv funcs, put then in a impl namespace with priv in front
namespace DEEP_BLUE_GENOME {

// TODO use these containers instead of manually working with sorted vectors: http://www.boost.org/doc/libs/1_55_0/doc/html/container/non_standard_containers.html#container.non_standard_containers.flat_xxx
class NotFoundException : public std::runtime_error {
public:
	NotFoundException(const std::string& kind)
	:	runtime_error(kind)
	{
	}
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

/**
 * Make unique and sorted
 */
template <class T>
void erase_duplicates(T& container) {
	boost::erase(container, boost::unique<boost::return_found_end>(boost::sort(container)));
}

} // end namespace

#if GCC_VERSION <= 40800
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif



