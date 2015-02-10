// Author: Tim Diels <timdiels.m@gmail.com>

#include "Gene.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

Gene::Gene()
:	id(0), gene_collection_id(0), ortholog_group_id(mysqlpp::null)
{
}

Gene::Gene(GeneId id, GeneCollectionId gene_collection_id, NullableOrthologGroupId ortholog_group_id, const std::string& name)
:	id(id), gene_collection_id(gene_collection_id), ortholog_group_id(ortholog_group_id), name(name)
{
}

GeneId Gene::get_id() const {
	return id;
}

GeneCollectionId Gene::get_gene_collection_id() const {
	return gene_collection_id;
}

bool Gene::has_orthologs() const {
	return !ortholog_group_id.is_null;
}

OrthologGroupId Gene::get_ortholog_group_id() const {
	assert(has_orthologs());
	return ortholog_group_id.data;
}

std::string Gene::get_name() const {
	return name;
}

bool Gene::operator<(const Gene& other) const {
	return id < other.id;
}

} // end namespace
