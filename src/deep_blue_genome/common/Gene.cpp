// Author: Tim Diels <timdiels.m@gmail.com>

#include "Gene.h"
#include <deep_blue_genome/common/Database.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

Gene::Gene()
:	id(0), gene_collection_id(0), ortholog_group_id(mysqlpp::null)
{
}

Gene::Gene(GeneId id, GeneCollectionId gene_collection_id, const std::string& name, NullableOrthologGroupId ortholog_group_id)
:	id(id), gene_collection_id(gene_collection_id), name(name), ortholog_group_id(ortholog_group_id)
{
}

Gene::Gene(GeneId id, Database& database)
:	id(id)
{
	auto query = database.prepare("SELECT gene_collection_id, name FROM gene WHERE id = %0q");
	query.parse();
	auto result = query.store(id);

	if (result.num_rows() == 0) {
		throw NotFoundException((make_string() << "Failed to find gene with id " << id).str());
	}

	assert(result.num_rows() == 1);
	auto row = *result.begin();

	gene_collection_id = row[0];
	name = row[1].conv<std::string>("");
	ortholog_group_id = row[2].conv<decltype(ortholog_group_id)>(mysqlpp::null);
}

GeneId Gene::get_id() const {
	return id;
}

GeneCollectionId Gene::get_gene_collection_id() const {
	return gene_collection_id;
}

std::string Gene::get_name() const {
	return name;
}

bool Gene::operator<(const Gene& other) const {
	return id < other.id;
}

bool Gene::has_orthologs() const {
	return !ortholog_group_id.is_null;
}

OrthologGroupId Gene::get_ortholog_group_id() const {
	assert(has_orthologs());
	return ortholog_group_id.data;
}

} // end namespace
