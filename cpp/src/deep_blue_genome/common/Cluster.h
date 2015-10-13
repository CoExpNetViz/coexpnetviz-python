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

#include <deep_blue_genome/common/Serialization.h>

namespace DEEP_BLUE_GENOME {

class Gene;

/**
 * A cluster of a Clustering
 */
class Cluster
{
public:
	typedef std::vector<Gene*> Genes;

public:
	Cluster(const std::string& name);

	void add(Gene&);
	std::string get_name() const;

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

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	Cluster();

private:
	Genes genes;
	std::string name;
};

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Cluster::serialize(Archive& ar, const unsigned int version) {
	ar & genes;
	ar & name;
}

}
