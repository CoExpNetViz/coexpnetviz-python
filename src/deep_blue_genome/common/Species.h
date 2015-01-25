// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <boost/regex.hpp>
#include <deep_blue_genome/common/util.h>

namespace DEEP_BLUE_GENOME {

class Database;
class GeneMapping;
class GeneDescriptions;
class GeneExpressionMatrix;

/**
 * A species
 */
class Species {
public:
	typedef std::vector<std::string>::const_iterator name_iterator;

	Species(std::string name, Database&);

	/**
	 * Get name of species
	 */
	std::string get_name() const;

	/**
	 * Get names of gene expression matrices of species
	 */
	Iterable<name_iterator> get_gene_expression_matrices() const;

	// TODO when the database gets updated with new expression matrices or generic clusterings, specific clusterings for all those combos should be added. (that way morph also needn't worry about it)
	/**
	 * Get clusterings specific to gene expression matrix
	 */
	Iterable<name_iterator> get_clusterings(std::string gene_expression_matrix) const;

	/**
	 * Set gene pattern.
	 *
	 * A gene is part of this species iff its name matches gene_pattern
	 *
	 * @param gene_pattern A regular expression of the language of genes part of this species
	 */
	void set_gene_pattern(std::string gene_pattern);

	const std::string& get_gene_pattern() const;
	const boost::regex& get_gene_pattern_re() const;

	/**
	 * Create or update gene expression matrix.
	 *
	 * Updating never removes any clusterings, it only adds more
	 */
	template <class IterableT>
	void update_gene_expression_matrix(std::string name, IterableT clustering_names);

	/**
	 * Get whether or not a gene web page pattern is known
	 */
	bool has_gene_web_page() const;

	void unset_gene_web_page() const;

	void set_gene_web_page(const std::string);
	const std::string& get_gene_web_page() const;

	bool has_gene_descriptions() const;
	void has_gene_descriptions(bool has_it);
	std::shared_ptr<GeneDescriptions> get_gene_descriptions() const;

	bool has_gene_mapping() const;
	void has_gene_mapping(bool has_it);
	std::shared_ptr<GeneMapping> get_gene_mapping() const; // TODO get via db

	std::shared_ptr<GeneExpressionMatrix> get_gene_expression_matrix(std::string name) const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::string name;
	Database& database;
	bool has_gene_descriptions_;
	bool has_gene_mapping_;
	std::unordered_map<std::string, std::vector<std::string>> gene_expression_matrices; // name of expression matrix mapped to a list of clusterings specific to that matrix
	std::vector<std::string> clusterings; // name of each clustering specific to a species, not to a gene expression matrix
	std::string gene_pattern; // regex string that matches any gene that belongs to this species
	boost::regex gene_pattern_re; // gene_pattern as regex
	std::string gene_web_page;

	std::vector<std::string> gene_expression_matrix_names; // redundant vectors because special iterators are hard to return sometimes...
};


/////////////////////
// hpp

template<class Archive>
void Species::serialize(Archive& ar, const unsigned int version) {
	ar & has_gene_descriptions_;
	ar & has_gene_mapping_;
	ar & gene_expression_matrices;
	ar & clusterings;
	ar & gene_pattern;
	if (Archive::is_loading::value) {
		set_gene_pattern(gene_pattern); // fills gene_pattern_re
	}
	ar & gene_web_page;
	ar & gene_expression_matrix_names;
}

template <class IterableT>
void Species::update_gene_expression_matrix(std::string name, IterableT clustering_names) {
	gene_expression_matrix_names.emplace_back(name); // TODO currently just adds, should merge
	gene_expression_matrices[name] = clustering_names; // TODO merge with current clustering names vec (Don't want duplicates. Validate input for duplicate names first)
}

}
