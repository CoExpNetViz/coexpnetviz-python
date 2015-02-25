// Author: Tim Diels <timdiels.m@gmail.com>

#include "SpliceVariant.h"
#include <deep_blue_genome/common/Gene.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

SpliceVariant::SpliceVariant()
{
}

SpliceVariant::SpliceVariant(Gene& gene, SpliceVariantId splice_variant_id)
:	gene(&gene), splice_variant_id(splice_variant_id)
{
}

Gene& SpliceVariant::get_gene() {
	return *gene;
}

SpliceVariantId SpliceVariant::get_splice_variant_id() const {
	return splice_variant_id;
}

Gene& SpliceVariant::as_gene() {
	ensure(splice_variant_id == 1,
			(make_string() << "Expected gene, got splice variant: " << *this).str(),
			ErrorType::SPLICE_VARIANT_INSTEAD_OF_GENE
	);
	return get_gene();
}

void SpliceVariant::print(std::ostream& str) const {
	str << "splice variant " << splice_variant_id << " of " << *gene;
}

GeneCollection& SpliceVariant::get_gene_collection() const {
	return gene->get_gene_collection();
}

} // end namespace
