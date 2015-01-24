// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <utility>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/ublas.h>

namespace DEEP_BLUE_GENOME {

/**
 * A cluster of a Clustering
 */
class Cluster //TODO: public boost::noncopyable
{
public:
	Cluster() {}  // boost::serialization uses this to construct an invalid Cluster before loading it
	Cluster(std::string name);

	void add(std::string gene);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	std::vector<std::string>::const_iterator begin() const;
	std::vector<std::string>::const_iterator end() const;
	std::vector<std::string>::iterator begin();
	std::vector<std::string>::iterator end();

	std::string get_name() const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::vector<std::string> genes;
	std::string name;

private:
	friend class boost::serialization::access;
};


/////////////////////
// hpp

template<class Archive>
void Cluster::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & genes;
}

} // end MORPHC
