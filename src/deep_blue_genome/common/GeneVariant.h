// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/Gene.h>

namespace DEEP_BLUE_GENOME {

// TODO it looks like .1 can be called a splice variant just as well. In which case a variant of NULL... does it ever make sense?
/**
 * Gene with all introns/exons (regular gene), or splice variant of gene (actual gene variant)
 *
 * Note: The 'transcript id' and 'splice variant' of a gene is equivalent.
 * The most common variant gets id=1.
 */
class GeneVariant
{
public:
	/**
	 * Construct invalid gene variant, a null value
	 */
	GeneVariant();

	GeneVariant(GeneVariantId, const Gene& gene, NullableSpliceVariantId);
	GeneVariant(GeneVariantId, Database&);

	GeneVariantId get_id() const;

	Gene get_gene() const;
	bool is_splice_variant() const;
	SpliceVariantId get_splice_variant_id() const;

	bool operator<(const GeneVariant&) const;

private:
	GeneVariantId id;
	Gene gene;
	NullableSpliceVariantId splice_variant_id;
};


} // end namespace


namespace std {
template <> struct hash<DEEP_BLUE_GENOME::GeneVariant>  // Template syntax, pretty as ever
{
	size_t operator()(const DEEP_BLUE_GENOME::GeneVariant& x) const
	{
		size_t hash = 0;
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_gene());
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_splice_variant_id());
		return hash;
	}
};
} // end namespace
