// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include "GeneExpression.h"
#include "Cluster.h"

namespace MORPHC {

/**
 * A clustering of gene expression data.
 *
 * Contains exactly all genes of its corresponding gene expression
 */
class Clustering
{
public:
	Clustering(std::string path, GeneExpression&);

	const std::vector<Cluster>& get_clusters() const;
	GeneExpression& get_source() const;

	std::string get_name() const {
		return name;
	}

private:
	std::string name;
	std::vector<Cluster> clusters;  // mutually disjunct clusters
	GeneExpression& gene_expression; // gene expression data we clustered
}; // TODO consider sorted vectors instead of sets

}
