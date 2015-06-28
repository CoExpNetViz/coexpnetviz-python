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

#include <boost/container/flat_set.hpp>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/GeneVariant.h>

namespace DEEP_BLUE_GENOME {

class OrthologGroup;
class SpliceVariant;

class Gene : public GeneVariant  // TODO conceptually a Gene is not a GeneVariant, variants of a Gene are gene variants. Probably just need to rename the base class to.. something else
{
public:
	/**
	 * Construct gene
	 *
	 * @param group Ortholog group the gene is part of
	 */
	Gene(const std::string& name, GeneCollection&);

	std::string get_name() const;

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

public:
	/**
	 * Get range of ortholog groups this gene is a part of
	 */
	auto get_ortholog_groups() const {
		return ortholog_groups;
	}

public: // Internal methods
	/**
	 * Internal method: add family to gene without adding gene to family
	 *
	 * @param group Ortholog group the gene is part of
	 */
	void add_ortholog_group(OrthologGroup& group) {
		ortholog_groups.emplace(&group);
	}

	/**
	 * Internal method: remove family from gene without removing gene from family
	 *
	 * @param group Ortholog group the gene is part of
	 */
	void remove_ortholog_group(OrthologGroup& group) {
		ortholog_groups.erase(&group);
	}

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
	boost::container::flat_set<OrthologGroup*> ortholog_groups; // ortholog groups this gene is part of.
	std::vector<std::unique_ptr<SpliceVariant>> splice_variants; // TODO stable_vector
};


} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Gene::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & gene_collection;
	ar & ortholog_groups;
	ar & splice_variants;
}

}
