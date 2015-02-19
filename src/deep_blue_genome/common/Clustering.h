// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <list>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/Cluster.h>

namespace DEEP_BLUE_GENOME {

#pragma db object
/**
 * A clustering of genes
 */
class Clustering : public boost::noncopyable
{
public:
	typedef std::vector<std::unique_ptr<Cluster>> Clusters;
	typedef Clusters::const_iterator const_iterator;

public:
	/**
	 * Load from database
	 */
	Clustering(ClusteringId);

	/**
	 * Construct a clustering from plain text file
	 *
	 * TODO describe plain text format
	 *
	 * @param expression_matrix If it can only be used with a specific matrix, specify it here, otherwise pass "".
	 */
	Clustering(const std::string& name, const std::string& path, const std::string& expression_matrix, Database&);

	/**
	 * Get iterator to first cluster
	 */
	const_iterator begin() const;
	const_iterator end() const;

	std::string get_name() const {
		return name;
	}

	void database_insert();

private:
	friend class odb::access;

	Clustering() {};  // for ODB

	#pragma db id auto
	ClusteringId id;

	#pragma db not_null
	std::shared_ptr<GeneCollection> gene_collection;

	#pragma db null
	std::shared_ptr<GeneExpressionMatrix> expression_matrix; // null if not associated with a matrix

	// TODO make value_not_null default for all containers if possible
	#pragma db value_not_null
	Clusters clusters;  // mutually disjunct clusters

	std::string name;
};


}
