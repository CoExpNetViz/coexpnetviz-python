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
#include "OrthologGroup.h"
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/OrthologGroup.h>

using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COMMON {
namespace WRITER {

YAML::Node write_yaml(const OrthologGroup& group) {
	YAML::Node node;

	for (auto& family_id : group.get_external_ids()) {
		YAML::Node family;
		family["source"] = family_id.get_source();
		family["id"] = family_id.get_id();
		node.push_back(family);
	}

	return node;
}


YAML::Node write_yaml_with_genes(const OrthologGroup& group) {
	YAML::Node node;
	node["external_ids"] = write_yaml(group);
	YAML::Node genes; // TODO can this be initialised to an empty list so we don't get ~ on output?
	for (auto gene : group.get_genes()) {
		genes.push_back(gene->get_name());
	}
	node["genes"] = genes;
	return node;
}

}}} // end namespace
