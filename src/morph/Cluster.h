// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <map>
#include <utility>
#include "GenesOfInterest.h"
#include "ublas.h"

class Cluster
{
public:
	void add(size_type index);

	/**
	 * Returns pair of (genes_of_interest, other_genes) in the cluster
	 */
	std::pair<indirect_array&, indirect_array&> get_genes(GenesOfInterest&) const;

	indirect_array& get_genes() const;

private:
	std::map<GenesOfInterest*, std::pair<std::vector<Gene*>, std::vector<Gene*>>> goi_genes; // TODO instead of maps you could use vectors and add an index field to GenesOfInterest
	std::vector<size_type> genes;
};
