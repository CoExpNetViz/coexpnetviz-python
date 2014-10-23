// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "util.h"

using namespace std;
namespace ublas = boost::numeric::ublas;

GeneExpression::GeneExpression(std::string path)
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
		int gene_index = 0;
		for (int i=0; i<expression_matrix.size1(); i++) {
			string gene_name;
			in >> gene_name;
			if (!in.eof() && !in.good()) {
				throw runtime_error("Syntax error in expression matrix: an empty line, an incomplete line, ...");
			}
			for (int j=0; j<expression_matrix.size2(); j++) {
				in >> expression_matrix(i, j);
				if (!in.eof() && !in.good()) {
					throw runtime_error("Syntax error in expression matrix: an empty line, an incomplete line, ...");
				}
			}
		}
	});

	// TODO gene_correlations
}

matrix& GeneExpression::get_gene_correlations() {
	return gene_correlations;
}

Gene& GeneExpression::get_gene(std::string name) {
	return genes.find(name)->second;
}
