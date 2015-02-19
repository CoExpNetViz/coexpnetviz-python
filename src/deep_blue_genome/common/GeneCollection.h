// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/GeneVariant.h>
#include <deep_blue_genome/common/GeneParserRule.h>

namespace DEEP_BLUE_GENOME {

class Database;

#pragma db object
/**
 * Identified genes in a genome
 *
 * All of which originate from a single data source, e.g. the rice RAP db
 */
class GeneCollection : public boost::noncopyable
{
public:
	GeneCollection(GeneCollectionId, Database&);

	GeneCollection(const std::string& name, const std::string& species, YAML::Node parser_rules, const NullableGeneWebPage& gene_web_page, Database&);

	std::string get_name() const;

	/**
	 * Get whether or not a gene web page pattern is known
	 */
	bool has_gene_web_page() const;
	std::string get_gene_web_page() const;

	/**
	 * Insert into database, also insert genome if it doesn't exist yet
	 */
	void database_insert();

	/**
	 * Overwrite gene collection id with this one's data
	 */
	void database_update(GeneCollectionId, Database&) const; // TODO assert(id==0)

	/**
	 * Get gene by name, inserts gene if it doesn't exist yet
	 *
	 * @throws NotFoundException if name doesn't match this gene collection
	 */
	GeneVariant get_gene_variant(const std::string& name); // TODO NotFoundException("Gene $name"),

	/**
	 * Returns true if name matches this gene collection
	 *
	 * Places resulting gene in out. Otherwise behaves the same as get_gene_by_name
	 */
	bool try_get_gene_variant(const std::string& name, GeneVariant& out);

private:
	friend class odb::access;

	GeneCollection() {};  // for ODB

	#pragma db id auto
	GeneCollectionId id;

	#pragma db unique
	std::string name;

	std::string species;

	NullableGeneWebPage gene_web_page;

	#pragma db value_not_null
	std::vector<std::unique_ptr<GeneParserRule>> gene_parser_rules;
};


} // end namespace
