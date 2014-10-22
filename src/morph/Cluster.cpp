// Author: Tim Diels <timdiels.m@gmail.com>

#include "Cluster.h"

using namespace std;
namespace ublas = boost::numeric::ublas;

pair<indirect_array, indirect_array> Cluster::get_genes(GenesOfInterest&) const {
	return make_pair(indirect_array(), indirect_array()); // TODO
}
