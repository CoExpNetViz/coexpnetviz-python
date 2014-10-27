// Author: Tim Diels <timdiels.m@gmail.com>

#include "Cluster.h"

using namespace std;
namespace ublas = boost::numeric::ublas;

Cluster::Cluster(string name)
:	name(name)
{
}

void Cluster::add(size_type gene_index) {
	assert(find(genes.begin(), genes.end(),gene_index) == genes.end());
	genes.emplace(gene_index);
}

bool Cluster::empty() const {
	return genes.empty();
}
const unordered_set<size_type>& Cluster::get_genes() const {
	return genes;
}

string Cluster::get_name() const {
	return name;
}
