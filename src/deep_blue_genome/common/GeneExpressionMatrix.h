// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <memory>
#include <deep_blue_genome/common/ublas.h>
#include <deep_blue_genome/common/Species.h>

// TODO size_type is unsigned long long or such, we don't need thaaat much. Should swap it for a typedef of our own and then set that to uint32_t. You probably won't need more than uint, but there's not much extra effort in using a typedef
namespace DEEP_BLUE_GENOME {

class GeneExpressionMatrixClustering;
class Database;

/**
 * Gene expression matrix
 *
 * matrix(i,j) = expression of gene i under condition j
 *
 * Note: these row indices are often used as gene ids in deep blue genome apps
 */
class GeneExpressionMatrix : public boost::noncopyable, public std::enable_shared_from_this<GeneExpressionMatrix>
{
public:
	/**
	 * Construct invalid expression matrix (for loading via serialization)
	 */
	GeneExpressionMatrix(std::string name, std::string species_name, Database&);

	/**
	 * Construct an expression matrix from a TODO particular plain text format
	 *
	 * @param species_name Name of species to which the gene expressions belong
	 */
	GeneExpressionMatrix(std::string name, std::string species_name, std::string path, Database&);

	~GeneExpressionMatrix() { std::cout << "~" << name << std::endl; }

	/**
	 * Get index of row corresponding to given gene
	 */
	size_type get_gene_index(std::string name) const;
	std::string get_gene_name(size_type gene_index) const;
	bool has_gene(std::string name) const;

	std::string get_name() const;
	std::string get_species_name() const;

	void dispose_expression_data();

	Iterable<Species::name_iterator> get_clusterings() const;
	std::shared_ptr<GeneExpressionMatrixClustering> get_clustering(std::string clustering_name);

	/**
	 * Get inner matrix representation
	 */
	const matrix& get() const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::string name; // name of dataset
	std::string species_name;
	Database& database;

	matrix expression_matrix; // row_major

	std::vector<size_type> genes; // sorted list of genes
	std::unordered_map<std::string, size_type> gene_indices; // all genes, name -> row index of gene in gene_correlations
	std::unordered_map<size_type, std::string> gene_names;
	std::unordered_map<size_type, size_type> gene_column_indices; // gene row index -> columng index of gene in gene_correlations
};


/////////////////////
// hpp

template<class Archive>
void GeneExpressionMatrix::serialize(Archive& ar, const unsigned int version) {
	ar & expression_matrix;
	ar & genes;
	ar & gene_indices;
	ar & gene_names;
	ar & gene_column_indices;
}

}
