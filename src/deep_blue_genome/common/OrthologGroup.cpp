// Author: Tim Diels <timdiels.m@gmail.com>

#include "OrthologGroup.h"
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/util/printer.h>
#include <boost/range.hpp>

using namespace std;

namespace DEEP_BLUE_GENOME {

OrthologGroup::OrthologGroup() // TODO biology would call these a family, not a group. Stick to bioinformatics terms
{
}

OrthologGroup::OrthologGroup(GeneFamilyId id) // TODO change all arg passing to what matches guidelines
{
	external_ids.emplace(std::move(id));
}

void OrthologGroup::set_iterator(OrthologGroup::DatabaseIterator it) {
	database_it = it;
}

void OrthologGroup::add(Gene& gene) {
	assert(contains(genes, &gene) || genes.size() == 0 || !is_singleton()); // can't add more than 1 gene to a singleton group

	gene.set_ortholog_group(*this);
	genes.emplace(&gene);
}

const OrthologGroup::Genes& OrthologGroup::get_genes() const {
	assert(!genes.empty()); // Did not fully initialise group
	return genes;
}

void OrthologGroup::merge(OrthologGroup&& other, Database& database) {
	assert(&other != this);

	// Set other genes to this group
	for (auto gene : other.genes) {
		gene->set_ortholog_group(*this);
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

bool OrthologGroup::is_singleton() const {
	return genes.size() == 1;
}

std::ostream& operator<<(std::ostream& out, const OrthologGroup& group) {
	out << "ortholog family {" << intercalate(";", group.get_external_ids()) << "}";
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

} // end namespace
