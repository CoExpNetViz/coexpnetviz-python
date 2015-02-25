// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <deep_blue_genome/common/Serialization.h>

namespace DEEP_BLUE_GENOME {

class Gene;

/**
 * A cluster of a Clustering
 */
class Cluster
{
public:
	typedef std::vector<Gene*> Genes;

public:
	Cluster(const std::string& name);

	void add(Gene&);
	std::string get_name() const;

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	Genes::const_iterator begin() const;
	Genes::const_iterator end() const;
	Genes::iterator begin();
	Genes::iterator end();

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	Cluster();

private:
	Genes genes;
	std::string name;
};

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Cluster::serialize(Archive& ar, const unsigned int version) {
	ar & genes;
	ar & name;
}

}
