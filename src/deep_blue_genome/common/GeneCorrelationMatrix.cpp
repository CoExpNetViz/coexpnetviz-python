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

#include <deep_blue_genome/common/stdafx.h>
#include "GeneCorrelationMatrix.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

/*GeneCorrelationMatrix::GeneCorrelationMatrix(const GeneExpressionMatrix& gene_expression_matrix, const boost::container::flat_set<GeneExpressionMatrixRow>& gene_indices)
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
}*/

const GeneCorrelationMatrix::MatrixType& GeneCorrelationMatrix::get() const {
	return gene_correlations;
}

GeneExpressionMatrixRow GeneCorrelationMatrix::get_column_index(GeneExpressionMatrixRow row_index) const {
	return row_to_column_indices.at(row_index);
}

GeneExpressionMatrixRow GeneCorrelationMatrix::get_row_index(GeneExpressionMatrixRow column_index) const {
	return column_to_row_indices.at(column_index);
}

} // end namespace

