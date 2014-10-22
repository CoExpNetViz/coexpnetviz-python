// Author: Tim Diels <timdiels.m@gmail.com>

#include "Ranking.h"

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

Ranking::Ranking(GenesOfInterest& g, Clustering& c)
:	genes_of_interest(g), clustering(c), rankings(c.get_source().get_gene_correlations().size1(), -1.0)
{
	rank_genes();
	rank_self();
}
// TODO define NDEBUG on release
void Ranking::rank_genes() {
	auto& gene_correlations = clustering.get_source().get_gene_correlations();
	for (auto& cluster : clustering.get_clusters()) {
		auto pair = cluster.get_genes(genes_of_interest);
		auto& interesting_genes = pair.first;
		auto& candidates = pair.second;
		auto sub_matrix = project(gene_correlations, candidates, interesting_genes);
		auto goi_count = interesting_genes.size();
		noalias(project(rankings, candidates)) = prod(sub_matrix, ublas::scalar_vector<double>(goi_count)) / goi_count;
	}
}

void Ranking::rank_self() {
	// TODO
	//for (gene : genes_of_interest) {
		// leave it out, rank, ausr thingy
	//}
}
