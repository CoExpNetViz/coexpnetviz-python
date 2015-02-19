// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/Gene.h>
#include <odb/lazy-ptr.hxx>

namespace DEEP_BLUE_GENOME {

#pragma db object
// TODO A variant of NULL... does it ever make sense?
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
	GeneVariant(); // TODO after ODB, do we still need this public?

	GeneVariant(GeneVariantId, const Gene& gene, NullableSpliceVariantId);
	GeneVariant(GeneVariantId, Database&);

	std::shared_ptr<Gene> get_gene() const;
	bool is_splice_variant() const;
	SpliceVariantId get_splice_variant_id() const;

	bool operator<(const GeneVariant&) const;

	void add_equivalent(odb::lazy_shared_ptr<GeneVariant>);

private:
	friend class odb::access;

	#pragma db id auto
	GeneVariantId id;

	#pragma db not_null
	std::shared_ptr<Gene> gene;

	#pragma db value_not_null unordered
	std::vector<odb::lazy_shared_ptr<GeneVariant>> equivalent_gene_variants; // Highly similar gene variants from other gene collections. There is some accuracy error incurred when using this, due to the mapping source (e.g. blast with some cut-offs)

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
