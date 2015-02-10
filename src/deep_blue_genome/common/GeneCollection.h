// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/regex.hpp>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/Gene.h>

namespace DEEP_BLUE_GENOME {

class Database;

/**
 * Identified genes in a genome
 *
 * All of which originate from a single data source, e.g. the rice RAP db
 */
class GeneCollection : public boost::noncopyable
{
public:
	GeneCollection(GeneCollectionId, Database&);

	GeneCollection(const std::string& name, const std::string& species, const std::string& gene_format_match, const std::string& gene_format_replace, const mysqlpp::sql_varchar_null& gene_web_page, Database&);

	GeneCollectionId get_id() const;
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
	Gene get_gene_by_name(const std::string& name); // TODO NotFoundException("Gene $name"),

	/**
	 * Returns true if name matches this gene collection
	 *
	 * Places resulting gene in out. Otherwise behaves the same as get_gene_by_name
	 */
	bool try_get_gene_by_name(const std::string& name, Gene& out);

private:
	void set_gene_format_match(const std::string& matcher);

private:
	GeneCollectionId id;
	std::string name;
	std::string species;

	std::string gene_format_match; // regex string that matches any gene that belongs to this gene collection
	boost::regex gene_format_match_re;

	std::string gene_format_replace; // used for s/gene_format_match/gene_format_replace/, for formatting gene names

	NullableGeneWebPage gene_web_page;

	Database& database;
};


} // end namespace
