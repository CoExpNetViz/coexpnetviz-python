// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

class GeneCollection;
class OrthologGroup;
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
 * Invariant: a gene is part of 0 or 1 ortholog group
 */
class Database : public boost::noncopyable {
public:
	typedef std::vector<std::string>::const_iterator name_iterator;

	/**
	 * Construct database
	 *
	 * The previous state of database at database_path will be loaded, if any.
	 *
	 * @param database_path location of database. Currently this is a config with paths to all db files
	 */
	Database(std::string database_path = "/home/limyreth/dbg_db"); // TODO don't hardcode, pass by arg

	void execute(const std::string& query);

	/**
	 * Erase everything in the database
	 */
	void clear();

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
	 * @returns created ortholog group
	 */
	OrthologGroup& add_ortholog_group(std::string external_id);

	/**
	 * Delete ortholog group
	 */
	void erase(OrthologGroup&);

	void add(std::unique_ptr<GeneCollection>&&);

	/**
	 * Save to disk
	 */
	void save();

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	/**
	 * Get path to file that contains the main data
	 */
	std::string get_main_file() const;

private:
	std::vector<std::unique_ptr<GeneCollection>> gene_collections;
	std::vector<std::unique_ptr<OrthologGroup>> ortholog_groups;

	std::string database_path;
};

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void Database::serialize(Archive& ar, const unsigned int version) {
	ar & gene_collections;
	ar & ortholog_groups;
}

}
