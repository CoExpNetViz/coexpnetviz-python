// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

Clustering::Clustering()
:	gene_collection(nullptr), expression_matrix(nullptr)
{
}

GeneCollection& Clustering::get_gene_collection() const {
	return *gene_collection;
}

Clustering::const_iterator Clustering::begin() const {
	return clusters.begin();
}

Clustering::const_iterator Clustering::end() const {
	return clusters.end();
}

std::string Clustering::get_name() const {
	return name;
}

}
