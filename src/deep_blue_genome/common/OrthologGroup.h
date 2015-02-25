// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <deep_blue_genome/common/Serialization.h>

namespace DEEP_BLUE_GENOME {

class Gene;
class Database;

/**
 * A group/set/cluster of orthologous genes
 *
 * Invariant: shall contain no duplicates (not to be confused with gene duplication)
 */
class OrthologGroup
{
public:
	typedef std::vector<Gene*> Genes;

	friend std::ostream& operator<<(std::ostream&, const OrthologGroup&);

public:
	OrthologGroup(std::string);

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

	Genes::const_iterator begin() const;
	Genes::const_iterator end() const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	OrthologGroup();

private:
	std::vector<std::string> external_ids; // Currently only plaza ids
	Genes genes;
};

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
