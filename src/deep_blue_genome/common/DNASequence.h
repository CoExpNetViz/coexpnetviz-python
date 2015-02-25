// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

class GeneVariant;

/**
 * A subsequence of DNA
 *
 * Invariant: shall contain no duplicates (not to be confused with gene duplication)
 */
class DNASequence
{
public:
	DNASequence(GeneVariant& owner);

	/**
	 * Add sequence that's highly similar to this one
	 *
	 * Silently fails when adding a sequence already present in list
	 */
	void add_highly_similar(DNASequence&);

	GeneVariant& get_owner();

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	DNASequence();

private:
	GeneVariant* owner; // we assume a sequence belongs to a Gene or a SpliceVariant
	std::vector<DNASequence*> highly_similar_sequences;  // 'highly' is determined by whatever mappings a user put in the database // TODO do the mappings we use pick up gene duplicates as well? I.e. on entirely different locations... i.e. does similarity also imply similar location?
};

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void DNASequence::serialize(Archive& ar, const unsigned int version) {
	ar & owner;
	ar & highly_similar_sequences;
}

}
