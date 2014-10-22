// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpression.h"
#include <fstream>

using namespace std;
namespace ublas = boost::numeric::ublas;

GeneExpression::GeneExpression(std::string path)
:	name(path)
{
	// TODO load
}

matrix& GeneExpression::get_gene_correlations() {
	return gene_correlations;
}
