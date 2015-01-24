// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <utility>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/ublas.h>

namespace DEEP_BLUE_GENOME {

/**
 * A cluster of a GeneExpressionMatrixClustering
 */
class GeneExpressionMatrixCluster //TODO: public boost::noncopyable
{
public:
	GeneExpressionMatrixCluster() {}  // boost::serialization uses this to construct an invalid Cluster before loading it
	GeneExpressionMatrixCluster(std::string name);

	void add(size_type index);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	std::vector<size_type>::const_iterator begin() const;
	std::vector<size_type>::const_iterator end() const;
	std::vector<size_type>::iterator begin();
	std::vector<size_type>::iterator end();

	std::string get_name() const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::vector<size_type> genes;
	std::string name;

private:
	friend class boost::serialization::access;
};


/////////////////////
// hpp

template<class Archive>
void GeneExpressionMatrixCluster::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & genes;
}

} // end MORPHC
