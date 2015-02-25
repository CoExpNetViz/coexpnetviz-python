// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpressionMatrixCluster.h"

using namespace std;

#if false
namespace DEEP_BLUE_GENOME {

GeneExpressionMatrixCluster::GeneExpressionMatrixCluster(string name)
:	name(name)
{
}

void GeneExpressionMatrixCluster::add(size_type gene_index) {
	genes.emplace_back(gene_index);
}

bool GeneExpressionMatrixCluster::empty() const {
	return genes.empty();
}

std::vector<size_type>::const_iterator GeneExpressionMatrixCluster::begin() const {
	return genes.begin();
}

std::vector<size_type>::const_iterator GeneExpressionMatrixCluster::end() const {
	return genes.end();
}

std::vector<size_type>::iterator GeneExpressionMatrixCluster::begin() {
	return genes.begin();
}

std::vector<size_type>::iterator GeneExpressionMatrixCluster::end() {
	return genes.end();
}

string GeneExpressionMatrixCluster::get_name() const {
	return name;
}

}
#endif
