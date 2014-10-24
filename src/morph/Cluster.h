// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <map>
#include <utility>
#include <unordered_set>
#include "GenesOfInterest.h"
#include "ublas.h"

class Cluster
{
public:
	Cluster(std::string name);

	void add(size_type index);

	const std::unordered_set<size_type>& get_genes() const;
	std::string get_name() const;

private:
	std::unordered_set<size_type> genes;
	std::string name;
};
