// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/StringMapping.h>

namespace MORPHC {

/**
 * Functional annotations
 */
class GeneDescriptions : public boost::noncopyable {
public:
	GeneDescriptions(std::string path);

	/**
	 * Get description of gene
	 */
	std::string get(std::string gene) const;

private:
	StringMapping mapping; // gene -> description
};

}
