// Author: Tim Diels <timdiels.m@gmail.com>

#include "Cluster.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

Cluster::Cluster(string name)
:	name(name)
{
}

void Cluster::add(GeneId gene_id) {
	genes.emplace_back(gene_id);
}

bool Cluster::empty() const {
	return genes.empty();
}

std::vector<GeneId>::const_iterator Cluster::begin() const {
	return genes.begin();
}

std::vector<GeneId>::const_iterator Cluster::end() const {
	return genes.end();
}

std::vector<GeneId>::iterator Cluster::begin() {
	return genes.begin();
}

std::vector<GeneId>::iterator Cluster::end() {
	return genes.end();
}

string Cluster::get_name() const {
	return name;
}

}
