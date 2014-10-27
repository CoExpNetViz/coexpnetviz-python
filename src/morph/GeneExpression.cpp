// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <gsl/gsl_statistics.h>
#include <cmath>
#include <iomanip>
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
	using namespace ublas;
	::indirect_array goi_indices(const_cast<size_type*>(all_goi.data()), const_cast<size_type*>(all_goi.data() + all_goi.size())); // TODO use this style everywhere instead of clumsily copying into unbounded array first

	gene_correlations = GeneCorrelations(expression_matrix.size1(), expression_matrix.size1(), expression_matrix.size1() * all_goi.size());

	// calculate Pearson's correlation
	// This is gsl_stats_correlation's algorithm, but in matrix form. It's thus also numerically stable
	size_type i;
	ublas::vector<long double> mean(expression_matrix.size1());
	ublas::vector<long double> delta(expression_matrix.size1());
	ublas::vector<long double> sum_sq(expression_matrix.size1(), 0.0); // sum of squares
	ublas::matrix<long double> sum_cross(expression_matrix.size1(), goi_indices.size(), 0.0);

	long double ratio;
	mean = column(expression_matrix, 0);

	for (i = 1; i < expression_matrix.size2(); ++i)
	{
		ratio = i / (i + 1.0);
		noalias(delta) = column(expression_matrix, i) - mean;
		sum_sq = sum_sq + element_prod(delta, delta) * ratio;
		sum_cross = sum_cross + outer_prod(delta, project(delta, goi_indices)) * ratio;
		mean = mean + delta / (i + 1.0);
	}

	transform(sum_sq.begin(), sum_sq.end(), sum_sq.begin(), ::sqrt);
	project(gene_correlations, ::indirect_array::all(), goi_indices) = element_div(sum_cross, outer_prod(sum_sq, project(sum_sq, goi_indices)));

	// TODO rm debug
	// Ad-hoc test to verify correctness of above algorithm
	/*for (size_type i=0; i<expression_matrix.size1(); i++) {
		for (auto j : all_goi) {
			auto a = gene_correlations(i,j);
			auto b = gsl_stats_correlation(&expression_matrix(i,0), 1, &expression_matrix(j,0), 1, expression_matrix.size2());
			if (abs((a - b)/b) > 1e-15) {
				cout << i << " " << j << endl;
				throw runtime_error((make_string() << setprecision(18) << a << " - " << b << " = " << abs((a - b)/b) << " > " << 1e-15).str());
			}
		}
	}*/
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

string GeneExpression::get_name() const {
	return name;
}

const std::vector<size_type>& GeneExpression::get_genes() const {
	return genes;
}
