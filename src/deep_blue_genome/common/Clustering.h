// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/Cluster.h>

namespace DEEP_BLUE_GENOME {

class GeneCollection;
class GeneExpressionMatrix;

/**
 * A clustering of genes
 */
class Clustering : public boost::noncopyable
{
public:
	typedef std::vector<Cluster> Clusters;
	typedef Clusters::const_iterator const_iterator;

	friend class DataFileImport;

public:
	/**
	 * Construct empty clustering
	 */
	Clustering();

	/**
	 * Get iterator to first cluster
	 */
	const_iterator begin() const;
	const_iterator end() const;

	std::string get_name() const;

	GeneCollection& get_gene_collection() const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	GeneCollection* gene_collection; // not null
	GeneExpressionMatrix* expression_matrix; // nullable
	Clusters clusters;  // mutually disjunct clusters
	std::string name;
};


}


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Clustering::serialize(Archive& ar, const unsigned int version) {
	ar & gene_collection;
	ar & expression_matrix;
	ar & clusters;
	ar & name;
}

}
