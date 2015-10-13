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

#include <deep_blue_genome/common/database_all.h>

namespace DEEP_BLUE_GENOME {

namespace MORPH {

/**
 * Genes of interest (GOI)
 *
 * A collection of genes.
 */
class GenesOfInterest : private boost::noncopyable
{
public:
	/**
	 * @param name Name of goi
	 * @param path Path to plain text
	 */
	GenesOfInterest(Database&, std::string name, std::string path);
	std::string get_name() const;

public:
	/**
	 * Get Range(Gene&); the baits
	 */
	auto get_genes() const {
		return genes | boost::adaptors::indirected;
	}

private:
	std::string name;
	boost::container::flat_set<Gene*> genes;
};

}}
