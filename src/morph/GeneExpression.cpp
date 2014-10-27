// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <gsl/gsl_statistics.h>
#include "util.h"


using namespace std;
namespace ublas = boost::numeric::ublas;

GeneExpression::GeneExpression(std::string path)
:	name(path)
{
	// load expression_matrix
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto current = begin;

		// count lines in file
		int line_count = count(begin, end, '\n') + 1; // #lines = #line_separators + 1

		// parse header
		std::vector<std::string> header_items;
		phrase_parse(current, end, *(lexeme[+(char_ - space)]) > eol, blank, header_items);
		int column_count = header_items.size();

		// resize matrix
		expression_matrix.resize(line_count-1, column_count-1, false);

		// parse gene lines
		int i=-1;
		int j=-1;
		auto on_new_gene = [this, &i, &j](std::string name) { // start new line
			if (i>=0 && j!=expression_matrix.size2()-1) {
				throw runtime_error("Incomplete line");
			}
			i++;
			if (!gene_indices.emplace(name, i).second) {
				throw runtime_error("Duplicate gene");
			}
			gene_names.emplace(i, name);
			genes.push_back(i);
			j=-1;
		};
		auto on_gene_value = [this, &i, &j](double value) { // gene expression value
			j++;
			expression_matrix(i, j) = value;
		};
		phrase_parse(current, end, (as_string[lexeme[+(char_ - space)]][on_new_gene] > (+double_[on_gene_value])) % eol, blank);
		if (j!=expression_matrix.size2()-1) {
			throw runtime_error("Incomplete line");
		}
		return current;
	});
}

void GeneExpression::generate_gene_correlations(const std::vector<size_type>& all_goi) {
	gene_correlations = GeneCorrelations(expression_matrix.size1(), expression_matrix.size1(), expression_matrix.size1() * all_goi.size());
	for (size_type i=0; i<expression_matrix.size1(); i++) {
		for (auto j : all_goi) {
			if (i==j) {
				gene_correlations(i,j) = 1.0;
			}
			else {
				gene_correlations(i,j) = gsl_stats_correlation(&expression_matrix(i,0), 1, &expression_matrix(j,0), 1, expression_matrix.size2());
			}
		}
	}
}

GeneCorrelations& GeneExpression::get_gene_correlations() {
	return gene_correlations;
}

size_type GeneExpression::get_gene_index(std::string name) const {
	assert(has_gene(name));
	return gene_indices.find(name)->second;
}

std::string GeneExpression::get_gene_name(size_type index) const {
	assert(gene_names.find(index) != gene_names.end());
	return gene_names.find(index)->second;
}

bool GeneExpression::has_gene(string gene) const {
	return gene_indices.find(gene) != gene_indices.end();
}

// TODO
void GeneExpression::debug() {
	//cout << "expr_mat(AT3G29810, 0) = " << expression_matrix(get_gene_index("AT3G29810"), 0) << endl;
	//cout << "cor(AT2G26660, AT3G29810) = " << gene_correlations(get_gene_index("AT2G26660"), get_gene_index("AT3G29810")) << endl;
}

string GeneExpression::get_name() const {
	return name;
}

const std::vector<size_type>& GeneExpression::get_genes() const {
	return genes;
}
