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

OrthologGroup::OrthologGroup(GeneFamilyId id)
{
	external_ids[id.get_source()].emplace(std::move(id));
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

void OrthologGroup::merge(OrthologGroup& other, Database& database) {
	// merge external ids
	for (auto& p : other.external_ids) {
		auto& group = external_ids[p.first];
		boost::insert(group, p.second);
	}

	// merge genes
	boost::insert(genes, other.genes);

	for (auto gene : genes) {
		gene->set_ortholog_group(*this);
	}

	// erase other
	database.erase(other.database_it);
}

bool OrthologGroup::is_singleton() const {
	return genes.size() == 1;
}

std::ostream& operator<<(std::ostream& out, const OrthologGroup& group) {
	out << "ortholog family {" << intercalate(";", group.get_external_ids()) << "}";
	return out;
}

std::vector<GeneFamilyId> OrthologGroup::get_external_ids() const {
	vector<GeneFamilyId> ids;
	for (auto p : external_ids) {
		boost::push_back(ids, p.second);
	}
	return ids;
}

const OrthologGroup::ExternalIdsGrouped& OrthologGroup::get_external_ids_grouped() const {
	return external_ids;
}

} // end namespace
