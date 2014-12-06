// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <map>
#include <utility>
#include <unordered_set>
#include "config/GenesOfInterest.h"
#include "ublas.h"
#include <morphc/serialization.h>

namespace MORPHC {

class Cluster
{
public:
	Cluster() {} // needed for serialization
	Cluster(std::string name);

	void add(size_type index);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	const std::unordered_set<size_type>& get_genes() const;
	std::string get_name() const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::unordered_set<size_type> genes;
	std::string name;
};


/////////////////////
// hpp

template<class Archive>
void Cluster::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & genes;
}

}
