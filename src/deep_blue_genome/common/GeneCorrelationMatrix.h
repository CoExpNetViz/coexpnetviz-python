// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <map>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>

namespace DEEP_BLUE_GENOME {

typedef matrix GeneCorrelations;

/**
 * Gene correlation matrix
 */
class GeneCorrelationMatrix : public boost::noncopyable
{
public:
	/**
	 * Create correlation matrix of all expression_matrix's genes and the given genes.
	 *
	 * Size of resulting matrix: (expression_matrix.row_count(), gene_indices.size()).
	 * matrix(i,j) = pearson_r(gene expr i, gene expr j)
	 *
	 * @param expression_matrix matrix whose gene expressions will be used to calculate the correlations
	 * @param gene_indices expression_matrix's gene indices to compare all genes with.
	 * 		Must be a subset of expression_matrix's gene indices.
	 */
	GeneCorrelationMatrix(const GeneExpressionMatrix& expression_matrix, const std::vector<GeneExpressionMatrixRow>& gene_indices);

	/**
	 * Get inner matrix
	 */
	const matrix& get() const;

	/**
	 * Get column index of corresponding to given row_index (if any, segfaults otherwise)
	 */
	GeneExpressionMatrixRow get_column_index(GeneExpressionMatrixRow row_index) const;

	GeneExpressionMatrixRow get_row_index(GeneExpressionMatrixRow column_index) const;

private:
	matrix gene_correlations; // TODO column major
	std::unordered_map<GeneExpressionMatrixRow, GeneExpressionMatrixRow> row_to_column_indices; // gene row index -> column index
	std::vector<GeneExpressionMatrixRow> column_to_row_indices;
};

}
