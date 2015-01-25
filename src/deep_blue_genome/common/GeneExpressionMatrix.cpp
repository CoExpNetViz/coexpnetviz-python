// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpressionMatrix.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <gsl/gsl_statistics.h>
#include <cmath>
#include <iomanip>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/TabGrammarRules.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;
namespace ublas = boost::numeric::ublas;

namespace DEEP_BLUE_GENOME {

GeneExpressionMatrix::GeneExpressionMatrix(string name, std::string species_name, Database& database)
:	name(name), species_name(species_name), database(database)
{
}

GeneExpressionMatrix::GeneExpressionMatrix(string name, std::string species_name, std::string path, Database& database)
:	GeneExpressionMatrix(name, species_name, database)
{
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

size_type GeneExpressionMatrix::get_gene_index(std::string name) const {
	assert(has_gene(name));
	return gene_indices.find(name)->second;
}

std::string GeneExpressionMatrix::get_gene_name(size_type index) const {
	assert(gene_names.find(index) != gene_names.end());
	return gene_names.find(index)->second;
}

bool GeneExpressionMatrix::has_gene(string gene) const {
	return gene_indices.find(gene) != gene_indices.end();
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

Iterable<Species::name_iterator> GeneExpressionMatrix::get_clusterings() const {
	return database.get_species(species_name)->get_clusterings(name);
}

std::string GeneExpressionMatrix::get_species_name() const {
	return species_name;
}

std::shared_ptr<GeneExpressionMatrixClustering> GeneExpressionMatrix::get_clustering(std::string clustering_name) {
	return database.get_gene_expression_matrix_clustering(shared_from_this(), clustering_name);
}

} // end namespace
