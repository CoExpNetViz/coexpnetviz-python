// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneExpressionMatrixCluster.h>
#include <deep_blue_genome/common/Clustering.h>

namespace DEEP_BLUE_GENOME {

// TODO attempt to simplify clustering made specific to GeneExpressionMatrix
// TODO attempt to simplify many 'performance' decisions we made before actually profiling (i.e. check whether they truly gain us any decent performance)

/**
 * A clustering specific to a GeneExpressionMatrix.
 *
 * Contains exactly all genes of its corresponding gene expression matrix
 */
class GeneExpressionMatrixClustering : public boost::noncopyable
{
public:
	typedef std::vector<GeneExpressionMatrixCluster> Clusters;
	typedef Clusters::const_iterator const_iterator;

public:
	/**
	 * Create invalid clustering for loading via serialization
	 */
	GeneExpressionMatrixClustering(std::shared_ptr<GeneExpressionMatrix>, std::string name);

	/**
	 * Create clustering specific to expression matrix from generic clustering
	 */
	GeneExpressionMatrixClustering(std::shared_ptr<GeneExpressionMatrix>, const Clustering& generic_clustering);

	/**
	 * Get iterator to first cluster
	 */
	const_iterator begin() const;
	const_iterator end() const;

	GeneExpressionMatrix& get_source() const;

	std::string get_name() const;

private:
	std::string name;
	Clusters clusters;  // mutually disjunct clusters
	std::shared_ptr<GeneExpressionMatrix> gene_expression_matrix; // gene expression data we clustered
};


}
