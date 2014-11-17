// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <map>
#include <utility>
#include <unordered_set>
#include "config/GenesOfInterest.h"
#include "ublas.h"

namespace MORPHC {

class Cluster
{
public:
	Cluster(std::string name);

	void add(size_type index);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	const std::unordered_set<size_type>& get_genes() const;
	std::string get_name() const;

private:
	std::unordered_set<size_type> genes;
	std::string name;
};

}
