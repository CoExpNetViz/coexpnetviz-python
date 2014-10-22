// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"

Clustering::Clustering(std::string path, GeneExpression& gene_expression)
:	name(path), source(gene_expression)
{
	// TODO load
}

const std::vector<Cluster>& Clustering::get_clusters() const {
	return clusters;
}

GeneExpression& Clustering::get_source() const {
	return source;
}
