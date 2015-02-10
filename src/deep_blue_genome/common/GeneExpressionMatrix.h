// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <memory>
#include <deep_blue_genome/common/ublas.h>
#include <deep_blue_genome/common/types.h>

// TODO size_type is unsigned long long or such, we don't need thaaat much. Should swap it for a typedef of our own and then set that to uint32_t. You probably won't need more than uint, but there's not much extra effort in using a typedef
namespace DEEP_BLUE_GENOME {

class GeneExpressionMatrixClustering;
class Database;

typedef uint32_t GeneExpressionMatrixRow;

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
	 * Load from database
	 */
	GeneExpressionMatrix(ExpressionMatrixId matrix_id, Database&);

	/**
	 * Construct an expression matrix from a TODO particular plain text format
	 *
	 * @param species_name Name of species to which the gene expressions belong
	 */
	GeneExpressionMatrix(const std::string& name, const std::string& path, Database&);

	/**
	 * Get index of row corresponding to given gene
	 */
	GeneExpressionMatrixRow get_gene_row(GeneId) const;
	GeneId get_gene_id(GeneExpressionMatrixRow) const;
	bool has_gene(GeneId) const;

	std::string get_name() const;
	std::string get_species_name() const;

	void dispose_expression_data();

	/**
	 * Get inner matrix representation
	 */
	const matrix& get() const;

	/**
	 * Insert into database
	 */
	void database_insert();

private:
	GeneExpressionMatrixId id;
	std::string name; // name of dataset
	GeneCollectionId gene_collection_id;
	Database& database;

	matrix expression_matrix; // row_major

	std::unordered_map<GeneExpressionMatrixRow, GeneId> gene_row_to_id;
	std::unordered_map<GeneId, GeneExpressionMatrixRow> gene_id_to_row;
};

}
