// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

/**
 * A cluster of a Clustering
 */
class Cluster
{
public:
	Cluster() {}  // boost::serialization uses this to construct an invalid Cluster before loading it
	Cluster(std::string name);

	void add(GeneId gene);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	std::vector<GeneId>::const_iterator begin() const;
	std::vector<GeneId>::const_iterator end() const;
	std::vector<GeneId>::iterator begin();
	std::vector<GeneId>::iterator end();

	std::string get_name() const;

private:
	std::vector<GeneId> genes;
	std::string name;
};

} // end MORPHC
