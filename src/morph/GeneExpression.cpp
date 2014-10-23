// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <gsl/gsl_statistics.h>
#include "util.h"

using namespace std;
namespace ublas = boost::numeric::ublas;

GeneExpression::GeneExpression(std::string path, const std::vector<std::string>& all_genes_of_interest)
:	name(path)
{
	// load expression_matrix
	read_file(path, [this](ifstream& in) {
		string line;

		// count lines in file
		int line_count = 0;
		while (getline(in, line)) {
			line_count++;
		}
		in.clear(); // clear eof
		in.seekg(0);

		// count number of columns (including gene name column)
		getline(in, line);
		std::vector<string> header_items;
		boost::split(header_items, line, boost::is_any_of("\t "));
		int column_count = header_items.size();

		// size matrix
		expression_matrix.resize(line_count-1, column_count-1, false);

		// fill matrix
		for (int i=0; i<expression_matrix.size1(); i++) {
			string gene_name;
			in >> gene_name;
			if (!in.eof() && !in.good()) {
				throw runtime_error("Syntax error in expression matrix: an empty line, an incomplete line, ...");
			}

			if (!gene_indices.emplace(gene_name, i).second) {
				throw runtime_error("Duplicate gene in expression matrix");
			}

			for (int j=0; j<expression_matrix.size2(); j++) {
				in >> expression_matrix(i, j);
				if (!in.eof() && !in.good()) {
					throw runtime_error("Syntax error in expression matrix: an empty line, an incomplete line, ...");
				}
			}
		}
	});

	load_correlations(all_genes_of_interest);
}

void GeneExpression::load_correlations(const std::vector<std::string>& all_genes_of_interest) {
	// TODO gene_correlations: this matrix is probably quite sparse. Is it beneficial to store it as sparse?

	// all_genes_of_interest -> row_indices
	indirect_array interesting_indices(all_genes_of_interest.size());
	for (int i=0; i<all_genes_of_interest.size(); i++) {
		interesting_indices(i) = get_gene_index(all_genes_of_interest.at(i));
	}

	gene_correlations.resize(expression_matrix.size1());
	for (size_type i=0; i<4 && i<expression_matrix.size1(); i++) {
		for (auto j : interesting_indices) {
			if (i==0) cout << ".";
			if (i==j) {
				gene_correlations(i,j) = 1.0;
			}
			else {
				gene_correlations(i,j) = gsl_stats_correlation(&expression_matrix(i,0), 1, &expression_matrix(j,0), 1, expression_matrix.size2());
			}
		}
	}
	cout << endl;
	throw runtime_error("dbg");
}

GeneCorrelations& GeneExpression::get_gene_correlations() {
	return gene_correlations;
}

size_type GeneExpression::get_gene_index(std::string name) {
	auto it = gene_indices.find(name);
	assert(it != gene_indices.end());
	return it->second;
}
