// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <unordered_map>
#include "GeneExpression.h"
#include "Cluster.h"
#include "config/Clustering.h"

namespace MORPHC {

/**
 * A clustering of gene expression data.
 *
 * Contains exactly all genes of its corresponding gene expression
 */
class Clustering
{
public:
	typedef std::unordered_map<std::string, Cluster> ClusterMap;
	typedef ClusterMap::const_iterator const_iterator;

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

private:
	std::string name;
	ClusterMap clusters;  // mutually disjunct clusters
	std::shared_ptr<GeneExpression> gene_expression; // gene expression data we clustered
}; // TODO consider sorted vectors instead of sets

}
