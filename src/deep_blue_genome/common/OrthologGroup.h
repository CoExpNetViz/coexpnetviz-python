// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <list>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/GeneFamilyId.h>

namespace DEEP_BLUE_GENOME {

class Gene;
class Database;

/**
 * A group/set/cluster of orthologous genes
 *
 * Invariant: shall contain no duplicates (not to be confused with gene duplication)
 *
 * To construct a valid object you need to first construct it, and then supply iterator to group via set_iterator.
 */
class OrthologGroup : private boost::noncopyable // TODO There's nothing specific to orthologs in here, could rename it to... GeneFamily?
{
public:
	typedef boost::container::flat_set<Gene*> Genes; // Turns out flat_set is more efficient than unordered_set with its insertions, up to about 500000 elements
	typedef boost::container::flat_map<std::string, boost::container::flat_set<GeneFamilyId>> ExternalIdsGrouped;
	typedef std::vector<GeneFamilyId> ExternalIds;
	typedef typename std::list<std::unique_ptr<OrthologGroup>>::iterator DatabaseIterator;

	friend std::ostream& operator<<(std::ostream&, const OrthologGroup&);

public:
	/**
	 * Construct singleton group
	 *
	 * Singleton groups are used as a default for genes not part of any other group
	 */
	OrthologGroup();

	OrthologGroup(GeneFamilyId);

	void set_iterator(OrthologGroup::DatabaseIterator);

	/**
	 * Add orthologous gene
	 *
	 * Also sets the inverse link on Gene to this.
	 *
	 * Silently fails when adding a sequence already present in list.
	 */
	void add(Gene&);

	/**
	 * Merge other group into this one
	 *
	 * Note: this removes the other group from database
	 */
	void merge(OrthologGroup&, Database&);

	/**
	 * Get range of external ids assigned to this ortholog group
	 *
	 * @returns range of ids
	 */
	ExternalIds get_external_ids() const;

	/**
	 * Get range of external ids grouped by source
	 *
	 * @returns range of pairs of (source, range of ids)
	 */
	const ExternalIdsGrouped& get_external_ids_grouped() const;

	/**
	 * Get range of all genes in group
	 */
	const Genes& get_genes() const;

	/**
	 * Get whether it's a singleton group.
	 *
	 * Note: group is singleton iff it has 1 gene
	 */
	bool is_singleton() const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	ExternalIdsGrouped external_ids; // Note: why no multimap? Multimap allows duplicate (key,value) pairs. Note: can be empty, e.g. in the singleton case
	Genes genes;
	DatabaseIterator database_it; // iterator to self in database that owns the group
};

/**
 * Write debug representation
 */
std::ostream& operator<<(std::ostream&, const OrthologGroup&);

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void OrthologGroup::serialize(Archive& ar, const unsigned int version) {
	ar & external_ids;
	ar & genes;
}

}
