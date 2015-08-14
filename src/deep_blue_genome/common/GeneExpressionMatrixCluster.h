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

#include <deep_blue_genome/common/ublas.h>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

/**
 * A cluster of a GeneExpressionMatrixClustering
 */
class GeneExpressionMatrixCluster : private boost::noncopyable
{
public:
	typedef std::vector<GeneExpressionMatrixRow> Rows;

	GeneExpressionMatrixCluster(std::string name);
	GeneExpressionMatrixCluster(GeneExpressionMatrixCluster&&);

	void add(GeneExpressionMatrixRow index);

	/**
	 * Get whether cluster is empty
	 */
	bool empty() const;

	/**
	 * Get iterator to first gene
	 */
	Rows::const_iterator begin() const;
	Rows::const_iterator end() const;
	Rows::iterator begin();
	Rows::iterator end();

	std::string get_name() const;

private:
	Rows genes;
	std::string name;
};

} // end MORPHC
