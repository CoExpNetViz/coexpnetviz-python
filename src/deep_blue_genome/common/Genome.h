// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>

namespace DEEP_BLUE_GENOME {
// TODO for now we needn't know which genome a gene collection is of, might as well leave it out now
/**
 * Genome of a species.
 */
class Genome : public boost::noncopyable
{
public:

private:
	uint32_t id;
	std::string species;
	//TODO sequence
};


} // end namespace
