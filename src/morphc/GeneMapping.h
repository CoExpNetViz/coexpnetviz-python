// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <morphc/StringMapping.h>
#include <boost/noncopyable.hpp>

namespace MORPHC {

/**
 * Maps gene names to other gene names (for mapping between different naming schemes)
 */
class GeneMapping : public boost::noncopyable {
public:
	GeneMapping(std::string path);

	/**
	 * Get mapped name of gene
	 */
	std::string get(std::string gene) const;

	/**
	 * Whether mapping is present
	 */
	bool has(std::string gene) const;

private:
	StringMapping mapping; // ns1:gene -> ns2:gene
};

}
