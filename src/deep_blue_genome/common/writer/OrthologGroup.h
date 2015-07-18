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
 * Writers of ortholog groups
 */

#pragma once

#include <yaml-cpp/yaml.h>
#include <iostream>
#include <boost/container/set.hpp>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <deep_blue_genome/common/GeneFamilyId.h>
#include <deep_blue_genome/common/OrthologGroup.h>
#include <deep_blue_genome/util/printer.h>

namespace DEEP_BLUE_GENOME {

class OrthologGroup; // TODO all classes in common should be in COMMON namespace

namespace COMMON {
namespace WRITER {

/**
 * Get group as yaml with all its external ids
 */
YAML::Node write_yaml(const OrthologGroup&);

/**
 * Like write_yaml, but with list of genes of the group
 */
YAML::Node write_yaml_with_genes(const OrthologGroup&);

/**
 * Get human readable format identifying the family
 */
auto format_long_id(const OrthologGroup& family) {
	using namespace std;
	using namespace boost;
	using namespace boost::adaptors;

	typedef std::pair<std::string, boost::container::flat_set<GeneFamilyId>> IdSubset;
	auto get_id_string = std::bind(&GeneFamilyId::get_id, std::placeholders::_1);
	auto get_ids_string = make_function([&get_id_string](const IdSubset& p){
		return make_printer([&p, &get_id_string](ostream& out){
			out << "from " << p.first << ": " << intercalate(", ", p.second | transformed(get_id_string));
		});
	});

	if (family.is_merged()) {
		// TODO write a concatenate function
		return intercalate_("",
			"Merged family { ",
			intercalate("; ", family.get_external_ids_grouped() | transformed(get_ids_string)),
			" }"
		);
	}
	else {
		return make_printer([&family](ostream& out) {
			auto&& id = *boost::begin(family.get_external_ids());
			out << "Family " << id.get_id() << " from " << id.get_source();
		});
	}
}

}}} // end namespace
