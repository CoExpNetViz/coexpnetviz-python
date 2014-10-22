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
		while (in.good()) {
			string gene_name;
			in >> gene_name;
			if (in.fail()) {
				break;
			}
			for (int i=0; i < expression_matrix.size2(); i++) {
				in >> expression_matrix(gene_index, i);
				if (in.fail()) {
					throw runtime_error("Incomplete line in expression matrix");
				}
			}
			gene_index++;
		}
	});
	cout << endl;

	// TODO gene_correlations
}

matrix& GeneExpression::get_gene_correlations() {
	return gene_correlations;
}

Gene& GeneExpression::get_gene(std::string name) {
	return genes.find(name)->second;
}
