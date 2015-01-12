// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/ublas.h>
#include <deep_blue_genome/common/Cache.h>

// TODO size_type is unsigned long long or such, we don't need thaaat much. Should swap it for a typedef of our own and then set that to uint32_t. You probably won't need more than uint, but there's not much extra effort in using a typedef
namespace MORPHC {

/**
 * Gene expression matrix
 *
 * matrix(i,j) = expression of gene i under condition j
 *
 * Note: these row indices are often used as gene ids in deep blue genome apps
 */
class GeneExpression : public boost::noncopyable // TODO rename to *Matrix
{
public:
	/**
	 * Load gene expression from file
	 */
	GeneExpression(std::string data_root, const YAML::Node, Cache&);

	/**
	 * Get index of row corresponding to given gene
	 */
	size_type get_gene_index(std::string name) const;
	std::string get_gene_name(size_type gene_index) const;
	bool has_gene(std::string name) const;

	std::string get_name() const;

	/**
	 * Get all gene indices, sorted
	 */
	const std::vector<size_type>& get_genes() const;

	void dispose_expression_data();

	/**
	 * Get inner matrix representation
	 */
	const matrix& get() const;


	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	/**
	 * Load gene expression from plain text file
	 */
	void load_plain(std::string path);

private:
	std::string name; // name of dataset

	matrix expression_matrix; // row_major

	std::vector<size_type> genes; // sorted list of genes
	std::unordered_map<std::string, size_type> gene_indices; // all genes, name -> row index of gene in gene_correlations
	std::unordered_map<size_type, std::string> gene_names;
	std::unordered_map<size_type, size_type> gene_column_indices; // gene row index -> columng index of gene in gene_correlations
};


/////////////////////
// hpp

template<class Archive>
void GeneExpression::serialize(Archive& ar, const unsigned int version) {
	ar & expression_matrix;
	ar & genes;
	ar & gene_indices;
	ar & gene_names;
	ar & gene_column_indices;
}

}
