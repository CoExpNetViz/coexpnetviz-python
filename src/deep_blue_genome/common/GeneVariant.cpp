// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneVariant.h"
#include <deep_blue_genome/common/Database.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

GeneVariant::GeneVariant()
:	id(0)
{
}

GeneVariant::GeneVariant(GeneVariantId id, const Gene& gene, NullableSpliceVariantId splice_variant_id)
:	id(id), gene(gene), splice_variant_id(splice_variant_id)
{
}

GeneVariant::GeneVariant(GeneVariantId id, Database& database)
:	id(id)
{
	auto query = database.prepare("SELECT gene_id, splice_variant_id, ortholog_group_id FROM gene_variant WHERE id = %0q");
	query.parse();
	auto result = query.store(id);

	if (result.num_rows() == 0) {
		throw NotFoundException((make_string() << "Failed to find gene variant with id " << id).str());
	}

	assert(result.num_rows() == 1);
	auto row = *result.begin();

	gene = Gene(row[0].conv<GeneId>(0), database);
	splice_variant_id = row[1].conv<decltype(splice_variant_id)>(mysqlpp::null);
}

GeneVariantId GeneVariant::get_id() const {
	return id;
}

bool GeneVariant::operator<(const GeneVariant& other) const {
	return id < other.id;
}

Gene GeneVariant::get_gene() const {
	return gene;
}

bool GeneVariant::is_splice_variant() const {
	return !splice_variant_id.is_null;
}

SpliceVariantId GeneVariant::get_splice_variant_id() const {
	return splice_variant_id.data;
}

} // end namespace
