// Author: Tim Diels <timdiels.m@gmail.com>

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
	assert(row_to_gene.find(row) != row_to_gene.end());
	return *row_to_gene.find(row)->second;
}

bool GeneExpressionMatrix::has_gene(const Gene& gene) const {
	return gene_to_row.find(const_cast<Gene*>(&gene)) != gene_to_row.end();
}

string GeneExpressionMatrix::get_name() const {
	return name;
}

void GeneExpressionMatrix::dispose_expression_data() {
	expression_matrix.resize(0, 0);
}

const matrix& GeneExpressionMatrix::get() const {
	return expression_matrix;
}

} // end namespace
