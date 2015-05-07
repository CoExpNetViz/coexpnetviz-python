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
