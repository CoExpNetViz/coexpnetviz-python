// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <boost/noncopyable.hpp>

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

/**
 * A group of baits
 *
 * The grouping has no predefined meaning
 */
class BaitGroup : public boost::noncopyable
{
public:
	BaitGroup(std::string name);

	void set_colour(std::string colour);
	std::string get_colour() const;

private:
	std::string name;
	std::string colour;
};


}} // end namespace
