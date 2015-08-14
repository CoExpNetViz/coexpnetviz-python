/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/util.h>

namespace DEEP_BLUE_GENOME {

typedef matrix GeneCorrelations;

/**
 * Gene correlation matrix
 */
class GeneCorrelationMatrix : private boost::noncopyable
{
public:
	typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::column_major> MatrixType;

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
	//GeneCorrelationMatrix(const GeneExpressionMatrix& expression_matrix, const boost::container::flat_set<GeneExpressionMatrixRow>& gene_indices);

	/**
	 * Get inner matrix
	 */
	const MatrixType& get() const;

	/**
	 * Get column index of corresponding to given row_index (if any, segfaults otherwise)
	 */
	GeneExpressionMatrixRow get_column_index(GeneExpressionMatrixRow row_index) const;

	GeneExpressionMatrixRow get_row_index(GeneExpressionMatrixRow column_index) const;

public:
	template <class GeneExpressionMatrixRowRange>
	GeneCorrelationMatrix(const GeneExpressionMatrix& gene_expression_matrix, const GeneExpressionMatrixRowRange& gene_indices)
	:	gene_correlations(gene_expression_matrix.get().size1(), boost::size(gene_indices))
	{
		using namespace std;
		namespace ublas = boost::numeric::ublas;
		using namespace ublas;

		auto& expression_matrix = gene_expression_matrix.get();
		DEEP_BLUE_GENOME::indirect_array gene_indices_(const_cast<GeneExpressionMatrixRow*>(&*gene_indices.begin()), const_cast<GeneExpressionMatrixRow*>(&*gene_indices.end()));

		for (auto gene : gene_indices) {
			row_to_column_indices[gene] = column_to_row_indices.size();
			column_to_row_indices.emplace_back(gene);
		}

		// calculate Pearson's correlation
		// This is gsl_stats_correlation's algorithm in matrix form.
		ublas::vector<long double> mean(expression_matrix.size1());
		ublas::vector<long double> delta(expression_matrix.size1());
		ublas::vector<long double> sum_sq(expression_matrix.size1(), 0.0); // sum of squares
		ublas::matrix<long double> sum_cross(expression_matrix.size1(), gene_indices_.size(), 0.0);

		mean = column(expression_matrix, 0);

		for (GeneExpressionMatrixRow i = 1; i < expression_matrix.size2(); ++i)
		{
			long double ratio = i / (i + 1.0);
			noalias(delta) = column(expression_matrix, i) - mean;
			sum_sq += element_prod(delta, delta) * ratio;
			sum_cross += outer_prod(delta, project(delta, gene_indices_)) * ratio;
			mean += delta / (i + 1.0);
		}

		transform(sum_sq.begin(), sum_sq.end(), sum_sq.begin(), ::sqrt);
		gene_correlations = element_div(sum_cross, outer_prod(sum_sq, project(sum_sq, gene_indices_)));
	}

private:
	MatrixType gene_correlations;
	std::unordered_map<GeneExpressionMatrixRow, GeneExpressionMatrixRow> row_to_column_indices; // gene row index -> column index
	std::vector<GeneExpressionMatrixRow> column_to_row_indices;
};


/////////////////////////////////////
// hpp



}
