// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/GeneParserRule.h>

// hpp includes
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>

namespace DEEP_BLUE_GENOME {

class Database;
class GeneExpressionMatrix;
class Clustering;

/**
 * Identified genes in a genome
 *
 * All of which originate from a single data source, e.g. the rice RAP db.
 *
 * There should never be 2 gene collection instances with the same name (in memory)
 *
 * Invariant: no 2 genes shall have the same name
 */
class GeneCollection : public boost::noncopyable, private boost::equality_comparable<GeneCollection>
{
public:
	/**
	 * Construct a gene collection
	 */
	GeneCollection(Database& database, const std::string& name, const std::string& species, YAML::Node parser_rules, const NullableGeneWebPage& gene_web_page);

	/**
	 * Construct an unknown gene collection, a dummy collection of genes (The NullGeneCollection if you like)
	 *
	 * Uses this gene parser:
	 * - match: "(.*)([.]([0-9]+))?"
	 * - replace: "$1"
	 * - splice_variant_group: 3
	 */
	GeneCollection(Database& database);

	/**
	 * You must call this after having deserialised a GeneCollection
	 */
	void init_serialised(Database& database);

	std::string get_name() const;
	boost::optional<std::string> get_gene_web_page() const;

	std::string get_species() const;

	/**
	 * Get gene variant by name
	 *
	 * A variant is created if it doesn't exist, but matches the naming scheme of this gene collection.
	 *
	 * @throws NotFoundException Variant doesn't match naming scheme of this gene collection
	 */
	GeneVariant& get_gene_variant(const std::string& name);

	/**
	 * Get gene variant
	 *
	 * Like get_gene_variant, but returns nullptr instead of throwing
	 */
	GeneVariant* try_get_gene_variant(const std::string& name);

	void add_clustering(std::unique_ptr<Clustering>&&);

	bool operator==(const GeneCollection&) const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	GeneCollection();

private:
	Database* database;
	bool is_unknown; // whether we are the unknown collection
	std::string name;
	std::string species;
	NullableGeneWebPage gene_web_page;
	std::unordered_map<std::string, std::unique_ptr<Gene>> name_to_gene; // gene name to gene, for all genes // TODO no unique_ptr needed (unless perhaps to move something around constructed elsewhere...; But could fix that by constructing it here first)
	std::vector<GeneParserRule> gene_parser_rules;
	std::unordered_map<std::string, std::unique_ptr<Clustering>> clusterings; // name -> clustering. General clusterings and those specific to a gene expression matrix

public:
	/**
	 * Get range of all genes in collection
	 *
	 * @return concept Range<Gene&>
	 */
	auto get_genes() const {
		return name_to_gene | boost::adaptors::map_values | boost::adaptors::indirected;
	}
};

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void GeneCollection::serialize(Archive& ar, const unsigned int version) {
	ar & is_unknown;
	ar & name;
	ar & species;
	ar & gene_web_page;
	ar & name_to_gene;
	ar & gene_parser_rules;
	ar & clusterings;
}

}
