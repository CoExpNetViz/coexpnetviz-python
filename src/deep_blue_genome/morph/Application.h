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
#include "Species.h"
#include <deep_blue_genome/common/Database.h>

namespace DEEP_BLUE_GENOME {
namespace MORPH {

class Application
{
public:
	Application(int argc, char** argv);
	void run();

private:
	void load_config();
	void load_jobs();

private:
	std::vector<Species> species; // list of species that need to be mined
	std::string config_path;
	std::string job_list_path;
	std::string output_path;
	std::unique_ptr<Database> database;
	int top_k;
	bool output_yaml;
};

}}
