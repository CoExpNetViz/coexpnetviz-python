// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/GeneMapping.h>

namespace DEEP_BLUE_GENOME {

class Database;

/**
 * Canonicalises genes of a species.
 *
 * A helper class.
 */
class Canonicaliser : public boost::noncopyable
{
public:
	Canonicaliser(Database&, const std::string& species);

	/**
	 * Get gene in its canonicalised form
	 *
	 * Note: Some alt genes specify a group of genes, so their canonical form is actually a set of genes
	 *
	 * @param gene Gene to canonise
	 * @return canonical names
	 */
	std::vector<std::string> get(std::string gene) const;

private:
	std::shared_ptr<GeneMapping> mapping;
	std::string species;
};


} // end namespace
