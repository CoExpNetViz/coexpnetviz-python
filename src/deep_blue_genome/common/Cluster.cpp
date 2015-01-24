// Author: Tim Diels <timdiels.m@gmail.com>

#include "Cluster.h"

using namespace std;
namespace ublas = boost::numeric::ublas;

namespace DEEP_BLUE_GENOME {

Cluster::Cluster(string name)
:	name(name)
{
}

void Cluster::add(string gene_index) {
	genes.emplace_back(gene_index);
}

bool Cluster::empty() const {
	return genes.empty();
}

std::vector<string>::const_iterator Cluster::begin() const {
	return genes.begin();
}

std::vector<string>::const_iterator Cluster::end() const {
	return genes.end();
}

std::vector<string>::iterator Cluster::begin() {
	return genes.begin();
}

std::vector<string>::iterator Cluster::end() {
	return genes.end();
}

string Cluster::get_name() const {
	return name;
}

}
