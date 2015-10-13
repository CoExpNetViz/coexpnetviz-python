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

#include <deep_blue_genome/common/stdafx.h>
#include "Database.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/common/database_all.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COMMON {
namespace READER {

void read_orthologs_yaml(Database& database, YAML::Node orthologs, const std::string& data_root) {
	DataFileImport importer(database);

	for (auto node : orthologs) {
		auto path = prepend_path(data_root, node["path"].as<string>());
		importer.add_orthologs(node["name"].as<string>(), path);
	}
}

}}} // end namespace
