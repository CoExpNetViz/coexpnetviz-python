// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include "GeneExpression.h"
#include "Cluster.h"

/**
 * A clustering of gene expression data.
 *
 * Some genes may be unclustered (not part of any cluster)
 */
class Clustering
{
public:
	Clustering(std::string path, GeneExpression&);

	const std::vector<Cluster>& get_clusters() const;
	GeneExpression& get_source() const;

	/**
	 * Get all gene indices present in this clustering
	 */
	const std::unordered_set<size_type>& get_genes() const;

private:
	std::string name;
	std::vector<Cluster> clusters;  // mutually disjunct clusters
	GeneExpression& gene_expression; // gene expression data we clustered
	std::unordered_set<size_type> genes;
}; // TODO consider sorted vectors instead of sets
