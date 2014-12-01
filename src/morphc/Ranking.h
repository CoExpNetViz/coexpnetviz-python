// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include "ublas.h"
#include "Clustering.h"

namespace MORPHC {

/**
 * Note: A negative ranking value for a gene means it wasn't ranked
 */
class Ranking
{
public:
	Ranking(std::vector<size_type> genes_of_interest, std::shared_ptr<Clustering>, std::string name);

	/**
	 * Save top k results in given directory
	 */
	void save(std::string directory, int top_k);

	bool operator>(const Ranking&) const;

private:
	void rank_genes(const std::vector<size_type>& genes_of_interest, boost::numeric::ublas::vector<double>& rankings);
	void rank_self();

private:
	std::vector<size_type> genes_of_interest; // genes_of_interest
	std::shared_ptr<Clustering> clustering;
	boost::numeric::ublas::vector<double> rankings; // size = genes.size(), gene_index -> ranking
	double ausr;
	std::string name;
};

}
