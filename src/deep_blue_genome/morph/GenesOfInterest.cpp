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

#include <deep_blue_genome/morph/stdafx.h>
#include "GenesOfInterest.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/TabGrammarRules.h>

using namespace std;
using boost::container::flat_set;

namespace DEEP_BLUE_GENOME {
namespace MORPH {

GenesOfInterest::GenesOfInterest(Database& database, string name, string path)
:	name(name)
{
	// Load
	read_file(path, [this, &database](const char* begin, const char* end) {
		auto on_field = [this, &database](const std::string& field) {
			assert(!field.empty());
			auto gene = database.try_get_gene(field);
			if (gene) {
				genes.emplace(gene);
			}
		};

		TabGrammarRules rules(true);
		parse(begin, end, rules.field[on_field] % (rules.line_separator | rules.field_separator));
		return begin;
	});

	// Expand bait set to include all genes highly similar to any of the given genes
	flat_set<Gene*> expanded_bait_set;
	for (auto&& gene : genes) {
		expanded_bait_set.emplace(gene);
		for (auto&& similar : gene->get_highly_similar_genes()) {
			expanded_bait_set.emplace(&similar);
		}
	}
	genes.swap(expanded_bait_set);
}

std::string GenesOfInterest::get_name() const {
	return name;
}

}}
