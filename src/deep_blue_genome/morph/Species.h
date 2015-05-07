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

#include <string>
#include <vector>
#include <memory>
#include <deep_blue_genome/common/Species.h>

namespace DEEP_BLUE_GENOME {

class Database;

namespace MORPH {

/**
 * A species
 */
class Species {
public:
	Species(std::string name, Database&);

	void add_goi(std::string name, std::string path);

	/**
	 * Find best ranking for each goi, save best rankings in output_path, save at most top_k genes of each ranking
	 */
	void run_jobs(std::string output_path, int top_k, bool output_yaml);

	std::string get_name() const;

private:
	std::shared_ptr<DEEP_BLUE_GENOME::Species> species;
	Database& database;
	std::vector<std::pair<std::string, std::string>> gois;
};

}}
