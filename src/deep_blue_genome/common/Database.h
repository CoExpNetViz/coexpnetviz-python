// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/Gene.h>
#include <boost/noncopyable.hpp>

namespace DEEP_BLUE_GENOME {

class GeneExpressionMatrix;
class Clustering;
class GeneCollection;

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
	mysqlpp::Query prepare(const std::string& query);

	/**
	 * Update database with new data
	 *
	 * TODO specify expected yaml format
	 */
	void update(std::string yaml_path); // TODO move this to cli/database

	std::shared_ptr<GeneExpressionMatrix> get_gene_expression_matrix(ExpressionMatrixId);

	std::shared_ptr<Clustering> get_clustering(ClusteringId);

	std::shared_ptr<GeneCollection> get_gene_collection(GeneCollectionId);

	/**
	 * @throws NotFoundException if doesn't exist
	 */
	ExpressionMatrixId get_gene_expression_matrix_id(GeneCollectionId, const std::string& name); // TODO rename ExpressionMatrixId -> Gene*

	/**
	 * Get gene by name, inserts gene if it doesn't exist yet
	 *
	 * @throws NotFoundException if name matches none of the gene collections
	 * @param name Name of gene
	 */
	Gene get_gene(const std::string& name);

	/**
	 * Get orthologs of gene, filtered by their gene collection
	 *
	 * @param gene_collections Orthologs of gene collections to include in result
	 */
	template <class GeneCollectionsIterable>
	std::vector<GeneId> get_orthologs(GeneId, const GeneCollectionsIterable&);

	/**
	 * Get gene collection id by name
	 */
	GeneCollectionId get_gene_collection_id(const std::string& name);

	/**
	 * Get gene by id
	 *
	 * @throws NotFoundException
	 */
	Gene get_gene(GeneId);

private:
   /**
	* Load instance of given Type.
	*
	* Ensures not having multiple instances of (Type, Id) in memory
	*/
	template <class Type, class Id>
	std::shared_ptr<Type> load(Id);

private:
	mysqlpp::TCPConnection connection;
	std::map<GeneCollectionId, std::shared_ptr<GeneCollection>> gene_collections;  // cache of all gene collections
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
}

}
