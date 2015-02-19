// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <odb/session.hxx>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/GeneVariant.h>

namespace DEEP_BLUE_GENOME {

class GeneExpressionMatrix;
class Clustering;
class GeneCollection;

// TODO could identify genes (variants) with/to NCBI database, and use their id and data (we probably should, and cache the things we need if necessary)
/**
 * Deep Blue Genome Database
 *
 * Invariant: No more than a single copy of data will be in memory.
 * (E.g. the same matrix won't be held in memory twice by Database.
 *  E.g. calling get_gene_descriptions twice in a row will return the same pointer)
 *
 * Data is only validated when being added to the database.
 *
 * Naming notes:
 * - update_* implies data will be added or changed, never removed.
 *
 * Caching policy: When updating, loaded objects and some next loads can still refer to the old version.
 * Anything returned as a shared_ptr is cached by the database.
 */
class Database : public boost::noncopyable {
public:
	typedef std::vector<std::string>::const_iterator name_iterator;

	/**
	 * @param database_path location of database. Currently this is a config with paths to all db files
	 */
	Database();

	void execute(const std::string& query);

	/**
	 * Update database with new data
	 *
	 * TODO specify expected yaml format
	 */
	void update(std::string yaml_path); // TODO move this to cli/database

	/**
	 * @throws NotFoundException if doesn't exist
	 */
	GeneExpressionMatrixId get_gene_expression_matrix_id(GeneCollectionId, const std::string& name); // TODO rename ExpressionMatrixId -> Gene*

	/**
	 * Get gene variant by name, inserts it (and its gene) if it doesn't exist yet
	 *
	 * @throws NotFoundException if name matches none of the gene collections
	 * @param name Name of gene
	 */
	std::shared_ptr<GeneVariant> get_gene_variant(const std::string& name);

	/**
	 * Get gene variant by gene
	 */
	GeneVariantId get_gene_variant_id(GeneId, NullableSpliceVariantId);

	/**
	 * Get orthologs filtered by their gene collection
	 *
	 * @param gene_collections Orthologs of gene collections to include in result
	 */
	template <class GeneCollectionsIterable>
	std::vector<GeneId> get_orthologs(OrthologGroupId, const GeneCollectionsIterable&);

	/**
	 * Get gene collection id by name
	 */
	GeneCollectionId get_gene_collection_id(const std::string& name);

	/**
	 * Get path to file in which expression matrix values are dumped
	 */
	std::string get_gene_expression_matrix_values_file(GeneExpressionMatrixId);

private:
   /**
	* Load instance of given Type.
	*
	* Ensures not having multiple instances of (Type, Id) in memory
	*/
	template <class Type, class Id>
	std::shared_ptr<Type> load(Id);

private:
	std::map<GeneCollectionId, std::shared_ptr<GeneCollection>> gene_collections;  // cache of all gene collections
	std::string storage_path; // where big blobs of data are stored (which don't belong inside the actual database itself)
	odb::session session;  // currently during the whole application run everything used so far, will remain in cache. We may need to change this later
};

} // end namespace


///////////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template <class Type, class Id>
std::shared_ptr<Type> Database::load(Id id) {
	using namespace std;
	static unordered_map<Id, weak_ptr<Type>> instances;

	// find instance
	auto it = instances.find(id);
	if (it == instances.end()) {
		it = instances.emplace(piecewise_construct, forward_as_tuple(id), forward_as_tuple()).first;
	}
	auto& instance = it->second;

	// get existing shared ptr
	if (auto ptr = instance.lock()) {
		return ptr;
	}

	// or make new shared ptr
	auto ptr = make_shared<Type>(id, *this);
	instance = ptr;
	return ptr;
}

template <class GeneCollectionsIterable>
std::vector<GeneId> Database::get_orthologs(OrthologGroupId group_id, const GeneCollectionsIterable& gene_collections) {
	assert(false);
	/*std::ostringstream str;
	str << "SELECT id FROM gene WHERE ortholog_group_id = " << group_id << " AND gene_collection_id NOT IN (";
	for (auto id : gene_collections) {
		str << id << ",";
	}
	str << 0 << ")"; // 0 is never used as an ortholog group id
	std::string str_ = str.str();
	str_.at(str_.length()-1) = ')';

	auto query = prepare(str_);
	auto result = query.store();

	if (result.num_rows() == 0) {
		throw NotFoundException("Ortholog group " + group_id);
	}

	std::vector<GeneId> orthologs;
	for (auto row : result) {
		orthologs.emplace_back(row[0]);
	}

	return orthologs;*/
}

} // end namespace
