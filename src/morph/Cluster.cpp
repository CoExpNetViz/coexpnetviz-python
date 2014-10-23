// Author: Tim Diels <timdiels.m@gmail.com>

#include "Cluster.h"

using namespace std;
namespace ublas = boost::numeric::ublas;

void Cluster::add(size_type gene_index) {
	assert(find(genes.begin(), genes.end(),gene_index) == genes.end());
	genes.push_back(gene_index);
}

pair<indirect_array&, indirect_array&> Cluster::get_genes(GenesOfInterest&) const {
	throw runtime_error("nope");
	//return make_pair(a, b); // TODO
}

indirect_array& Cluster::get_genes() const {
	throw runtime_error("nope");
	//return indirect_array(); // TODO
}
