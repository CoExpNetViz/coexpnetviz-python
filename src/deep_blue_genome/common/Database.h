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

#include <list>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/GeneCollection.h>
#include <deep_blue_genome/common/OrthologGroup.h>

// hpp includes
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <deep_blue_genome/util/functional.h>

namespace DEEP_BLUE_GENOME {

class GeneFamilyId;
class GeneVariant;

// TODO Pimpl pattern may help performance, i.e. operator new grabs from a pool
// TODO could identify genes (variants) with/to NCBI database, and use their id and data (we probably should, and cache the things we need if necessary)

// Notes on usage of std::unique_ptr:
// - Firstly, unique_ptr was needed to not invalidate pointers to items upon a capacity change, as these pointers are kept by many classes in the 'OO graph of data'.
// - Even for maps, a unique_ptr was needed as boost serialization throws 'pointer conflict' when the same object is serialized as reference and as pointer
// - std::unique_ptr inside containers is not supported inside containers by boost::serialization, but boost::shared_ptr is... And that's why we use boost::shared_ptr

/**
 * Deep Blue Genome Database
 *
 * Object oriented single-user single-threaded in-memory database.
 *
 * Constraints are supported through the validate() method.
 *
 * Note: names of data are stored case-sensitive, but any lookup by name is case-insensitive
 *
 * Gene families may overlap.
 *
 * Invariant: a gene is part of 0 or 1 ortholog group
 */
class Database : public boost::noncopyable { // TODO be consistent in the usage of ranges, and those should be ranges of &, not *. Furthermore, you want boost collections for collections of pointers; those have an interface of references
public:
	typedef std::vector<std::string>::const_iterator name_iterator;
	typedef std::list<std::unique_ptr<OrthologGroup>> OrthologGroups; // Note: pointer because of boost serialization: serialising something as pointer and elsewhere as reference = trouble // TODO it would be nice if boost serialization didn't have that restriction. There also are a bunch of std::move things that should be done to boost::serialization, like the ones we already have put in util/serialization

	/**
	 * Construct database
	 *
	 * The previous state of database at database_path will be loaded, if any.
	 *
	 * @param database_path location of database. Currently this is a config with paths to all db files
	 * @param start_fresh If false, load previous state, else start with empty database
	 */
	Database(std::string database_path, bool start_fresh=false);

	void execute(const std::string& query);

	/**
	 * Get gene variant by name, inserts it if it doesn't exist yet
	 *
	 * @throws NotFoundException if name matches none of the gene collections
	 * @param name Name of gene
	 */
	GeneVariant& get_gene_variant(const std::string& name);

	GeneVariant* try_get_gene_variant(const std::string& name);

	GeneCollection& get_gene_collection(std::string name);

	/**
	 * Create ortholog group
	 *
	 * @returns created group
	 */
	OrthologGroup& add_ortholog_group(const GeneFamilyId& external_id);

	/**
	 * @throws NotFoundException if doesn't exist
	 */
	GeneExpressionMatrix& get_gene_expression_matrix(std::string name);

	/**
	 * Add gene expression matrix
	 *
	 * @return the new matrix (now stored in database)
	 */
	GeneExpressionMatrix& add(std::unique_ptr<GeneExpressionMatrix>&& );

	/**
	 * Delete ortholog group
	 */
	void erase(OrthologGroups::iterator);

	void add(std::unique_ptr<GeneCollection>&&);

	/**
	 * Save to disk
	 */
	void save();

	/**
	 * Assert database integrity.
	 *
	 * This is a used to test for bugs in database code.
	 * You shouldn't need to use this in your algorithms.
	 */
	void verify();

public: // return type deduced funcs (can't be moved outside the class def)
	/**
	 * Get range of all families
	 */
	auto get_ortholog_groups() const {
		return ortholog_groups | boost::adaptors::indirected;
	}

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	/**
	 * Get set of all genes
	 */
	std::unordered_set<const Gene*> get_genes() const;

	/**
	 * Get path to file that contains the main data
	 */
	std::string get_main_file() const;

private:
	GeneCollection unknown_gene_collection; // a catch-all gene collection that collects genes that didn't match any of the known collections
	std::vector<std::unique_ptr<GeneCollection>> gene_collections; // TODO stable_vector, or ptr_vector from boost pointer container (e.g. if you find the compile time dependencies too harsh with non-pointer types; i.e. more includes)
	OrthologGroups ortholog_groups;
	std::unordered_map<std::string, std::unique_ptr<GeneExpressionMatrix>> gene_expression_matrices; // name -> matrix

	std::string database_path;
};

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Database::serialize(Archive& ar, const unsigned int version) {
	ar & unknown_gene_collection;
	ar & gene_expression_matrices;
	ar & gene_collections;
	for (auto& gene_collection : gene_collections) {
		gene_collection->init_serialised(*this);
	}

	ar & ortholog_groups;
	for (auto it = ortholog_groups.begin(); it != ortholog_groups.end(); it++) {
		(*it)->set_iterator(it);
	}
}

}
