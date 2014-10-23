// Author: Tim Diels <timdiels.m@gmail.com>

#include "Ranking.h"
#include "util.h"

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

Ranking::Ranking(std::vector<size_type>& goi, Clustering& clustering)
:	genes_of_interest(goi), clustering(clustering), rankings(clustering.get_source().get_gene_correlations().size1(), -1.0) // TODO use NaN instead
{
	rank_genes();
	rank_self();
}

// TODO define NDEBUG on release
void Ranking::rank_genes() {
	auto& gene_correlations = clustering.get_source().get_gene_correlations();
	for (auto& cluster : clustering.get_clusters()) {
		auto& cluster_genes = cluster.get_genes();

		// interesting_genes array
		::array interesting_genes_(genes_of_interest.size());
		auto in_cluster = [&cluster_genes](size_type gene) {
			return contains(cluster_genes, gene);
		};
		auto it = copy_if(genes_of_interest.begin(), genes_of_interest.end(), interesting_genes_.begin(), in_cluster);
		::indirect_array interesting_genes(distance(interesting_genes_.begin(), it), interesting_genes_);
		if (interesting_genes.size() == 0)
			continue;

		// candidate genes array
		::array candidates_(cluster_genes.size());
		auto is_not_gene_of_interest = [this](size_type gene) {
			return !contains(genes_of_interest, gene);
		};
		auto it2 = copy_if(cluster_genes.begin(), cluster_genes.end(), candidates_.begin(), is_not_gene_of_interest);
		::indirect_array candidates(distance(candidates_.begin(), it2), candidates_);
		if (candidates.size() == 0)
			continue;

		// compute rankings
		auto sub_matrix = project(gene_correlations, candidates, interesting_genes);
		auto goi_count = interesting_genes.size();
		noalias(project(rankings, candidates)) = prod(sub_matrix, ublas::scalar_vector<double>(goi_count)) / goi_count;
	}
	cout << rankings(0) << " ";
	cout << rankings(1) << " ";
	cout << rankings(2) << endl;
}

void Ranking::rank_self() {
	// TODO
	//for (gene : genes_of_interest) {
		// leave it out, rank, ausr thingy
	//}
}
