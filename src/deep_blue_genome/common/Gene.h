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
#include <deep_blue_genome/common/util.h>

namespace DEEP_BLUE_GENOME {

class OrthologGroup;
class GeneCollection;

class Gene : public boost::noncopyable
{
	friend OrthologGroup;

public:
	/**
	 * Construct gene
	 *
	 * @param group Ortholog group the gene is part of
	 */
	Gene(const std::string& name, GeneCollection&);

	std::string get_name() const;

	GeneCollection& get_gene_collection() const;

	void set_functional_annotation(std::string);
	boost::optional<std::string> get_functional_annotation();

	/**
	 * Add gene that's highly similar to this one
	 *
	 * Silently fails when adding a gene already present in list
	 */
	void add_highly_similar(Gene&);

public:
	/**
	 * Get Range(OrthologGroup*); range of ortholog groups this gene is a part of
	 */
	auto&& get_ortholog_groups() const {
		return ortholog_groups;
	}

	/**
	 * Get Range(Gene&); range of genes that are highly similar to this gene by DNA (or protein) sequence.
	 * 'highly' is determined by the user, by loading gene mappings denoting the highly similar.
	 */
	auto get_highly_similar_genes() const {
		return highly_similar_genes | boost::adaptors::indirected;
	}

private:
	/**
	 * Print short human readable description of self to stream
	 */
	void print(std::ostream&) const;
	friend std::ostream& operator<<(std::ostream&, const DEEP_BLUE_GENOME::Gene&);

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	Gene();

private: // Methods used by OrthologGroup
	/**
	 * Add family to gene without adding gene to family
	 *
	 * @param group Ortholog group the gene is part of
	 */
	void add_ortholog_group(OrthologGroup& group) {
		ortholog_groups.emplace(&group);
	}

	/**
	 * Remove family from gene without removing gene from family
	 *
	 * @param group Ortholog group the gene is part of
	 */
	void remove_ortholog_group(OrthologGroup& group) {
		ortholog_groups.erase(&group);
	}

private:
	// Note: we use pointers instead of references as Gene needs to be default constructible to be conveniently usable with boost serialization
	std::string name; // unique name of gene
	GeneCollection* gene_collection; // genes collection which this gene is part of. Not null
	boost::container::flat_set<OrthologGroup*> ortholog_groups; // ortholog groups this gene is part of.
	boost::optional<std::string> functional_annotation;
	std::vector<Gene*> highly_similar_genes;  // 'highly' is determined by whatever mappings a user put in the database
};

std::ostream& operator<<(std::ostream&, const Gene&);


} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Gene::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & gene_collection;
	ar & ortholog_groups;
	ar & functional_annotation;
	ar & highly_similar_genes;
}

}
