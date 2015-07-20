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

#include "OrthologGroup.h"
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/util/printer.h>
#include <boost/range.hpp>

using namespace std;

namespace DEEP_BLUE_GENOME {

OrthologGroup::OrthologGroup(GeneFamilyId id) // TODO change all arg passing to what matches C++ guidelines
{
	external_ids.emplace(std::move(id));
}

void OrthologGroup::set_iterator(OrthologGroup::DatabaseIterator it) {
	database_it = it;
}

void OrthologGroup::add(Gene& gene) {
	gene.add_ortholog_group(*this);
	genes.emplace(&gene);
}

const OrthologGroup::Genes& OrthologGroup::get_genes() const {
	assert(!genes.empty()); // Did not fully initialise group
	return genes;
}

void OrthologGroup::merge(OrthologGroup&& other, Database& database) {
	assert(&other != this);

	// Update other genes to this group
	for (auto gene : other.genes) {
		gene->remove_ortholog_group(other);
		gene->add_ortholog_group(*this);
	}

	// merge external ids
	// TODO could generalise this kind of destructive merge in an algorithm: i.e. swap the small for the big, then insert, perhaps reserve
	if (other.external_ids.size() > external_ids.size()) { // merge small into big, not the other way around
		external_ids.swap(other.external_ids);
	}
	external_ids.reserve(other.external_ids.size() + external_ids.size()); // it will be exactly this size
	boost::insert(external_ids, other.external_ids);

	// merge genes
	if (other.genes.size() > genes.size()) { // merge small into big, not the other way around
		genes.swap(other.genes);
	}
	genes.reserve(other.genes.size() + genes.size()); // this is just a worst case size
	boost::insert(genes, other.genes);

	// erase other
	database.erase(other.database_it);
}

std::size_t OrthologGroup::size() const {
	return genes.size();
}

std::ostream& operator<<(std::ostream& out, const OrthologGroup& group) {
	out << "OrthologFamily {" << intercalate(";", group.get_external_ids()) << "}";
	return out;
}

const OrthologGroup::ExternalIds& OrthologGroup::get_external_ids() const {
	return external_ids;
}

OrthologGroup::ExternalIdsGrouped OrthologGroup::get_external_ids_grouped() const {
	ExternalIdsGrouped ids;
	for (auto id : external_ids) {
		ids[id.get_source()].emplace(id);
	}
	return ids;
}

bool OrthologGroup::is_merged() const {
	return external_ids.size() > 1;
}

void OrthologGroup::erase(Database& database) {
	for (auto&& gene : genes) {
		gene->remove_ortholog_group(*this);
	}
	database.erase(database_it);
}

} // end namespace
