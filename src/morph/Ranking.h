// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include "ublas.h"
#include "Clustering.h"

/**
 * Note: A negative ranking value for a gene means it wasn't ranked
 */
class Ranking
{
public:
	Ranking(std::vector<size_type>& genes_of_interest, Clustering&);

private:
	void rank_genes();
	void rank_self();

private:
	std::vector<size_type>& genes_of_interest; // genes_of_interest
	Clustering& clustering;
	boost::numeric::ublas::vector<double> rankings; // size = genes.size(), gene_index -> ranking
};
