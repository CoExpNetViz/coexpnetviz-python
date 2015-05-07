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

#include "DNASequence.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/GeneVariant.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

DNASequence::DNASequence(GeneVariant& owner)
:	owner(&owner)
{
}

void DNASequence::add_highly_similar(DNASequence& sequence) {
	if (contains(highly_similar_sequences, &sequence))
		return;

	auto owner2 = &sequence.get_owner();
	ensure(&owner->get_gene_collection() != &owner2->get_gene_collection(),
			(make_string() << "Encountered sequence mapping between 2 genes of the same gene collection: " << *owner << ", " << *owner2).str(),
			ErrorType::GENERIC
	);

	highly_similar_sequences.emplace_back(&sequence);
}

GeneVariant& DNASequence::get_owner() {
	return *owner;
}

}
