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

/**
 * MCL file formats
 */

#pragma once

#include <yaml-cpp/yaml.h>

namespace DEEP_BLUE_GENOME {

class Database;

namespace COMMON {
namespace READER {

typedef std::vector<std::string> Cluster;

/**
 * Generic clustering
 *
 * Unlabeled clusters of items identified by string.
 */
class Clustering
{
public:
	/**
	 * Get range of Cluster in the clustering
	 */
	auto get_clusters() {
		return clusters;
	}

	void add_cluster(const Cluster&);

private:
	std::vector<Cluster> clusters;
};

class MCL
{
public:
	/**
	 * Read clustering outputted by an --abc run
	 */
	Clustering read_clustering(const std::string& path);
};

}}} // end namespace
