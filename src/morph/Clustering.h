// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include "GeneExpression.h"
#include "Cluster.h"

class Clustering
{
public:
	Clustering(std::string path, GeneExpression&);

	const std::vector<Cluster>& get_clusters() const;
	GeneExpression& get_source() const;

private:
	std::string name;
	std::vector<Cluster> clusters;
	GeneExpression& source; // gene expression data we clustered
};
