// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <list>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/Cluster.h>

namespace DEEP_BLUE_GENOME {

/**
 * A clustering of genes
 */
class Clustering : public boost::noncopyable
{
public:
	typedef std::vector<Cluster> Clusters;
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
	ClusteringId id;
	GeneCollectionId gene_collection_id;
	ExpressionMatrixId expression_matrix_id; // 0 if not associated with a matrix
	std::string name;
	Clusters clusters;  // mutually disjunct clusters
	Database& database;
};


}
