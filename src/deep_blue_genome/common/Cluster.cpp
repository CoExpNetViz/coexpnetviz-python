// Author: Tim Diels <timdiels.m@gmail.com>

#include "Cluster.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

Cluster::Cluster(string name)
:	name(name)
{
}

void Cluster::add(odb::lazy_shared_ptr<Gene> gene) {
	genes.emplace_back(gene);
}

bool Cluster::empty() const {
	return genes.empty();
}

Cluster::Genes::const_iterator Cluster::begin() const {
	return genes.begin();
}

Cluster::Genes::const_iterator Cluster::end() const {
	return genes.end();
}

Cluster::Genes::iterator Cluster::begin() {
	return genes.begin();
}

Cluster::Genes::iterator Cluster::end() {
	return genes.end();
}

string Cluster::get_name() const {
	return name;
}

}
