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

#pragma once

#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/GeneVariant.h>

namespace DEEP_BLUE_GENOME {

class Gene;

/**
 * Splice variant of gene
 *
 * Note: The 'transcript id' and 'splice variant' of a gene is equivalent.
 * The most common variant usually gets id=1.
 */
class SpliceVariant : public GeneVariant
{
public:
	SpliceVariant(Gene&, SpliceVariantId);

	SpliceVariantId get_splice_variant_id() const;

	GeneCollection& get_gene_collection() const;
	Gene& get_gene();
	Gene& as_gene();

protected:
	void print(std::ostream&) const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	SpliceVariant();

private:
	Gene* gene; // not null
	SpliceVariantId splice_variant_id;
};


} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void SpliceVariant::serialize(Archive& ar, const unsigned int version) {
	ar & gene;
	ar & splice_variant_id;
}

}
