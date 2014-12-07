// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <unordered_map>
#include "GeneExpression.h"
#include "Cluster.h"
#include "config/Clustering.h"
#include <morphc/serialization.h>
#include <boost/noncopyable.hpp>

namespace MORPHC {

/**
 * A clustering of gene expression data.
 *
 * Contains exactly all genes of its corresponding gene expression
 */
class Clustering : public boost::noncopyable
{
public:
	typedef std::vector<Cluster> Clusters;
	typedef Clusters::const_iterator const_iterator;

public:
	Clustering(CONFIG::Clustering, std::shared_ptr<GeneExpression>);

	/**
	 * Get iterator to first cluster
	 */
	const_iterator begin() const;
	const_iterator end() const;

	GeneExpression& get_source() const;

	std::string get_name() const {
		return name;
	}

	void load_plain(std::string path);

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::string name;
	Clusters clusters;  // mutually disjunct clusters
	std::shared_ptr<GeneExpression> gene_expression; // gene expression data we clustered
}; // TODO consider sorted vectors instead of sets


/////////////////////
// hpp

template<class Archive>
void Clustering::serialize(Archive& ar, const unsigned int version) {
	ar & clusters;
}


}
