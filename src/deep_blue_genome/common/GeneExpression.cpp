// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <gsl/gsl_statistics.h>
#include <cmath>
#include <iomanip>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/TabGrammarRules.h>

using namespace std;
namespace ublas = boost::numeric::ublas;

namespace MORPHC {

GeneExpression::GeneExpression(string data_root, const YAML::Node node, Cache& cache)
{
	name = node["name"].as<string>();

	// load expression_matrix
	cache.load_bin_or_plain(prepend_path(data_root, node["path"].as<string>()), *this);
}

void GeneExpression::load_plain(std::string path) {
	int j;
	read_file(path, [this, &j](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		auto current = begin;

		// count lines in file
		int line_count = count(begin, end, '\n') + 1; // #lines = #line_separators + 1

		// parse header
		std::vector<std::string> header_items;
		TabGrammarRules rules;
		parse(current, end, rules.line > eol, header_items);

		// resize matrix
		expression_matrix.resize(line_count-1, header_items.size()-1, false);

		// parse gene lines
		int i=-1;
		j=-1;
		auto on_new_gene = [this, &i, &j](std::string name) { // start new line
			to_lower(name);
			ensure(i<0 || j==expression_matrix.size2()-1, (
					make_string() << "Line " << i+2 << " (1-based, header included): expected "
					<< expression_matrix.size2() << " columns, got " << j+1).str(),
					ErrorType::GENERIC
			);
			i++;
			ensure(gene_indices.emplace(name, i).second,
					(make_string() << "Duplicate gene: " << name).str(),
					ErrorType::GENERIC);
			gene_names.emplace(i, name);
			genes.push_back(i);
			j=-1;
		};
		auto on_gene_value = [this, &i, &j](double value) { // gene expression value
			j++;
			expression_matrix(i, j) = value;
		};

		parse(current, end, (rules.field[on_new_gene] > rules.separator > (double_[on_gene_value] % rules.separator)) % eol);

		return current;
	});
	ensure(j == expression_matrix.size2()-1,
			(make_string() << "Error while reading " << path << ": Incomplete line: " << j+1 << " values instead of " << expression_matrix.size2()).str(),
			ErrorType::GENERIC);
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

void GeneExpression::dispose_expression_data() {
	expression_matrix.resize(0, 0);
}

const matrix& GeneExpression::get() const {
	return expression_matrix;
}

}
