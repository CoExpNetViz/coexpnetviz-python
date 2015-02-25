// Author: Tim Diels <timdiels.m@gmail.com>

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
