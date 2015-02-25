// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <utility>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/ublas.h>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

/**
 * A cluster of a GeneExpressionMatrixClustering
 */
class GeneExpressionMatrixCluster //TODO: public boost::noncopyable
{
public:
	typedef std::vector<GeneExpressionMatrixRow> Rows;

	GeneExpressionMatrixCluster(std::string name);

	void add(GeneExpressionMatrixRow index);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	Rows::const_iterator begin() const;
	Rows::const_iterator end() const;
	Rows::iterator begin();
	Rows::iterator end();

	std::string get_name() const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	GeneExpressionMatrixCluster() {} // TODO cpp

private:
	Rows genes;
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
