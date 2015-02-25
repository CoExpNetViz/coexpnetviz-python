// Author: Tim Diels <timdiels.m@gmail.com>

#include "OrthologGroup.h"
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

OrthologGroup::OrthologGroup()
{
}

OrthologGroup::OrthologGroup(std::string id)
{
	external_ids.emplace_back(std::move(id));
}

void OrthologGroup::add(Gene& gene) {
	if (contains(genes, &gene))
		return;

	gene.set_ortholog_group(*this);
	genes.emplace_back(&gene);
}

OrthologGroup::Genes::const_iterator OrthologGroup::begin() const {
	return genes.begin();
}

OrthologGroup::Genes::const_iterator OrthologGroup::end() const {
	return genes.end();
}

void OrthologGroup::merge(OrthologGroup& other, Database& database) {
	external_ids.insert(external_ids.begin(), other.external_ids.begin(), other.external_ids.end());
	sort(external_ids.begin(), external_ids.end());
	external_ids.erase(unique(external_ids.begin(), external_ids.end()), external_ids.end());

	genes.insert(genes.begin(), other.begin(), other.end());
	sort(genes.begin(), genes.end());
	genes.erase(unique(genes.begin(), genes.end()), genes.end());

	for (auto gene : genes) {
		gene->set_ortholog_group(*this);
	}

	database.erase(other);
}

std::ostream& operator<<(std::ostream& str, const OrthologGroup& group) {
	copy(group.external_ids.begin(), group.external_ids.end(), ostream_iterator<string>(str, ";"));
	return str;
}

} // end namespace
