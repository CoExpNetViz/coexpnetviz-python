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

namespace DEEP_BLUE_GENOME {

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
	GeneCollection(const std::string& name, const std::string& species, YAML::Node parser_rules, const NullableGeneWebPage& gene_web_page);

	std::string get_name() const;
	boost::optional<std::string> get_gene_web_page() const;

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

	/**
	 * @throws NotFoundException if doesn't exist
	 */
	GeneExpressionMatrix& get_gene_expression_matrix(const std::string& name);

	void add_gene_expression_matrix(std::unique_ptr<GeneExpressionMatrix>&& );
	void add_clustering(std::unique_ptr<Clustering>&&);

	bool operator==(const GeneCollection&) const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	GeneCollection();

private:
	std::string name;
	std::string species;
	NullableGeneWebPage gene_web_page;
	std::unordered_map<std::string, std::unique_ptr<Gene>> name_to_gene; // gene name to gene, for all genes
	std::vector<GeneParserRule> gene_parser_rules;
	std::unordered_map<std::string, std::unique_ptr<GeneExpressionMatrix>> gene_expression_matrices; // name -> matrix
	std::unordered_map<std::string, std::unique_ptr<Clustering>> clusterings; // name -> clustering. General clusterings and those specific to a gene expression matrix
};

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void GeneCollection::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & species;
	ar & gene_web_page;
	ar & name_to_gene;
	ar & gene_parser_rules;
	ar & gene_expression_matrices;
	ar & clusterings;
}

}
