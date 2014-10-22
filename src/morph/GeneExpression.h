// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include "ublas.h"

class GeneExpression
{
public:
	/**
	 * Load gene expression from file
	 */
	GeneExpression(std::string path);

	/**
	 * Get correlation matrix of genes (rows) and genes of interest (columns)
	 */
	matrix& get_gene_correlations(); // TODO only fill the correlation columns corresponding to a gene of interest

private:
	std::string name; // name of dataset

	//std::map<Gene*, row> gene_mappings; // Note: it's faster to have it stored directly on the gene object (though then you'd have to look up the correct GeneExpression)
	//matrix<Gene, Expression> of double, expression_matrix;

	//std::map<Gene*, row> gene_of_interest_mappings;
	matrix gene_correlations; // size = (size(genes), size(genes)), row = gene, column = gene of interest, val = correlation between 2 genes
};
