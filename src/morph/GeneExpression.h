// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <map>
#include "ublas.h"
#include "Gene.h"

typedef boost::numeric::ublas::mapped_matrix<double> GeneCorrelations;

class GeneExpression
{
public:
	/**
	 * Load gene expression from file
	 */
	GeneExpression(std::string path);

	void generate_gene_correlations(const std::vector<size_type>& all_goi);
	void debug();

	/**
	 * Get correlation matrix of genes (rows) and genes of interest (columns).
	 *
	 * Size of matrix: size(genes), size(genes)).
	 * mat(i,j) = correlation between gene with index i and gene with index j.
	 * mat(i,j) only has a valid value if j refers to a gene of interest.
	 */
	GeneCorrelations& get_gene_correlations(); // TODO only fill the correlation columns corresponding to a gene of interest

	size_type get_gene_index(std::string name) const;
	std::string get_gene_name(size_type index) const;
	bool has_gene(std::string name) const;

	std::string get_name() const;

private:
	void load_correlations(const std::vector<std::string>& all_genes_of_interest);

private:
	std::string name; // name of dataset

	matrix expression_matrix; // row_major
	GeneCorrelations gene_correlations;

	std::map<std::string, size_type> gene_indices; // all genes, name -> index of gene in matrices
	std::map<size_type, std::string> gene_names;
};
