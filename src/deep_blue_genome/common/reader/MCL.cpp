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
#include "MCL.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/TabGrammarRules.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COMMON {
namespace READER {

void Clustering::add_cluster(const Cluster& cluster) {
	clusters.emplace_back(cluster);
}

Clustering MCL::read_clustering(const std::string& path) {
	Clustering clustering;
	cout << "Loading MCL clustering '" << path << "'\n";

	read_file(path, [this, &clustering](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		// Assign ortholog groups
		auto on_line = [this, &clustering](const std::vector<std::string>& line) {
			// TODO validation
			clustering.add_cluster(line);
		};

		TabGrammarRules rules(true);
		parse(begin, end, rules.line[on_line] % eol);
		return begin;
	});
	return clustering;
}

}}} // end namespace
