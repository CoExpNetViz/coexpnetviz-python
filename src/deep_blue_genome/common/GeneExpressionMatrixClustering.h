/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

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
