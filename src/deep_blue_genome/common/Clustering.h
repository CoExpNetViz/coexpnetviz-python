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
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/Cluster.h>

namespace DEEP_BLUE_GENOME {

class GeneCollection;
class GeneExpressionMatrix;

/**
 * A clustering of genes
 */
class Clustering : public boost::noncopyable
{
public:
	typedef std::vector<Cluster> Clusters;
	typedef Clusters::const_iterator const_iterator;

	friend class DataFileImport;

public:
	/**
	 * Construct empty clustering
	 */
	Clustering();

	/**
	 * Get iterator to first cluster
	 */
	const_iterator begin() const;
	const_iterator end() const;

	std::string get_name() const;

	GeneCollection& get_gene_collection() const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	GeneCollection* gene_collection; // not null
	GeneExpressionMatrix* expression_matrix; // nullable
	Clusters clusters;  // mutually disjunct clusters
	std::string name;
};


}


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Clustering::serialize(Archive& ar, const unsigned int version) {
	ar & gene_collection;
	ar & expression_matrix;
	ar & clusters;
	ar & name;
}

}
