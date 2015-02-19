// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <odb/core.hxx>
#include <odb/lazy-ptr.hxx>
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

#pragma db object
/**
 * A cluster of a Clustering
 */
class Cluster
{
public:
	typedef std::vector<odb::lazy_shared_ptr<Gene>> Genes;

public:
	Cluster(std::string name);

	void add(odb::lazy_shared_ptr<Gene>);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	Genes::const_iterator begin() const;
	Genes::const_iterator end() const;
	Genes::iterator begin();
	Genes::iterator end();

	std::string get_name() const;

private:
	friend class odb::access;

	// TODO Pimpl pattern may help performance, i.e. operator new grabs from a pool
	Cluster() {}  // for ODB

	#pragma db id auto
	ClusterId id;

	#pragma db value_not_null
	Genes genes;

	#pragma db unique
	std::string name;
};

} // end MORPHC
