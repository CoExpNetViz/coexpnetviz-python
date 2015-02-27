// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/GeneVariant.h>

namespace DEEP_BLUE_GENOME {

class OrthologGroup;
class SpliceVariant;

class Gene : public GeneVariant
{
public:
	/**
	 * @param ortholog_group Ortholog group the gene is part of (Optional)
	 */
	Gene(const std::string& name, GeneCollection&, OrthologGroup*);

	std::string get_name() const;

	/**
	 * Get ortholog group
	 *
	 * @returns group if any, nullptr otherwise
	 */
	OrthologGroup* get_ortholog_group() const;

	/**
	 * Set ortholog group
	 *
	 * @throws if already part of an ortholog group
	 */
	void set_ortholog_group(OrthologGroup& group);

	/**
	 * Get splice variant
	 *
	 * The variant is created if it doesn't exist yet
	 *
	 * @throws NotFoundException Variant doesn't match naming scheme of this gene collection
	 */
	SpliceVariant& get_splice_variant(SpliceVariantId);

	GeneCollection& get_gene_collection() const;
	Gene& get_gene();
	Gene& as_gene();

protected:
	void print(std::ostream&) const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	Gene();

private:
	// Note: we use pointers instead of references as Gene needs to be default constructible to be conveniently usable with boost serialization
	std::string name; // unique name of gene
	GeneCollection* gene_collection; // genes collection which this gene is part of. Not null
	OrthologGroup* ortholog_group; // ortholog group this gene is part of. Nullable
	std::vector<std::unique_ptr<SpliceVariant>> splice_variants;
};


} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Gene::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & gene_collection;
	ar & ortholog_group;
	ar & splice_variants;
}

}