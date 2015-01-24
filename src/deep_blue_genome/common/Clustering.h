// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/Cluster.h>

namespace DEEP_BLUE_GENOME {

/**
 * A clustering of genes
 */
class Clustering : public boost::noncopyable
{
public:
	typedef std::vector<Cluster> Clusters;
	typedef Clusters::const_iterator const_iterator;

public:
	/**
	 * Construct invalid clustering (for loading via serialization)
	 */
	Clustering(std::string name);

	/**
	 * Construct a clustering from a TODO particular plain text format
	 */
	Clustering(std::string name, std::string path);

	/**
	 * Get iterator to first cluster
	 */
	const_iterator begin() const;
	const_iterator end() const;

	std::string get_name() const {
		return name;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::string name;
	Clusters clusters;  // mutually disjunct clusters
};


/////////////////////
// hpp

template<class Archive>
void Clustering::serialize(Archive& ar, const unsigned int version) {
	ar & clusters;
}


}
