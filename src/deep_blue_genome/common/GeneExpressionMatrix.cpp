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
#include <deep_blue_genome/common/GeneCollection.h>

using namespace std;
namespace ublas = boost::numeric::ublas;

namespace DEEP_BLUE_GENOME {

GeneExpressionMatrix::GeneExpressionMatrix(const string& name, const std::string& path, Database& database)
:	id(0), name(name), database(database)
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
		shared_ptr<GeneCollection> gene_collection;
		int i=-1; // row/line
		j=-1;
		auto on_new_gene = [this, &i, &j, &gene_collection](std::string name) { // start new line
			// TODO we currently assume gene is already in canonical form
			ensure(i<0 || j==expression_matrix.size2()-1, (
					make_string() << "Line " << i+2 << " (1-based, header included): expected "
					<< expression_matrix.size2() << " columns, got " << j+1).str(),
					ErrorType::GENERIC
			);
			i++;
			if (!gene_collection) {
				auto gene = this->database.get_gene(name);
				gene_collection_id = gene.get_gene_collection_id();
				gene_collection = this->database.get_gene_collection(gene_collection_id);
			}
			auto gene = gene_collection->get_gene_by_name(name);
			ensure(gene_id_to_row.emplace(gene.get_id(), i).second,
					(make_string() << "Duplicate gene: " << name).str(),
					ErrorType::GENERIC);
			gene_row_to_id[i] = gene.get_id();
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

GeneExpressionMatrixRow GeneExpressionMatrix::get_gene_row(GeneId gene_id) const {
	assert(has_gene(gene_id));
	return gene_id_to_row.find(gene_id)->second;
}

GeneId GeneExpressionMatrix::get_gene_id(GeneExpressionMatrixRow row) const {
	assert(gene_row_to_id.find(row) != gene_row_to_id.end());
	return gene_row_to_id.find(row)->second;
}

bool GeneExpressionMatrix::has_gene(GeneId gene) const {
	return gene_id_to_row.find(gene) != gene_id_to_row.end();
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

void GeneExpressionMatrix::database_insert() {
	// Insert expression_matrix
	auto stmt = database.prepare("INSERT INTO expression_matrix(name, gene_collection_id, rows, columns, values) VALUES (%0q, %1q, %2q, %3q, %4q)");
	stmt.parse();
	auto& array = expression_matrix.data();
	mysqlpp::sql_blob matrix_data(reinterpret_cast<const char*>(array.begin()), sizeof(double) * array.size()); // Let the hax begin TODO avoid hax // TODO try to avoid copy
	auto result = stmt.execute(name, gene_collection_id, expression_matrix.size1(), expression_matrix.size2(), matrix_data);
	ExpressionMatrixId matrix_id = result.insert_id();

	// Insert expression matrix rows
	stmt = database.prepare("INSERT INTO expression_matrix_row(matrix_id, row, gene_id) VALUES (%0q, %1q, %2q)");
	stmt.parse();
	for (auto& p : gene_id_to_row) {
		stmt.execute(matrix_id, p.second, p.first);
	}
}

} // end namespace
