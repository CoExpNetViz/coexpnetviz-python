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
#include "GeneExpressionMatrix.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

GeneExpressionMatrix::GeneExpressionMatrix()
{
}

GeneExpressionMatrixRow GeneExpressionMatrix::get_gene_row(Gene& gene) const {
	assert(has_gene(gene));
	return gene_to_row.find(&gene)->second;
}

Gene& GeneExpressionMatrix::get_gene(GeneExpressionMatrixRow row) const {
	return *row_to_gene.at(row);
}

bool GeneExpressionMatrix::has_gene(const Gene& gene) const {
	return gene_to_row.find(const_cast<Gene*>(&gene)) != gene_to_row.end();
}

string GeneExpressionMatrix::get_name() const {
	return name;
}

const matrix& GeneExpressionMatrix::get() const {
	return expression_matrix;
}

ostream& operator<<(ostream& out, const DEEP_BLUE_GENOME::GeneExpressionMatrix& matrix) {
	out << "gene expression matrix '" << matrix.get_name() << "'";
	return out;
}

} // end namespace

