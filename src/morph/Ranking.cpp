// Author: Tim Diels <timdiels.m@gmail.com>

#include "Ranking.h"
#include "util.h"
#include "gsl/gsl_statistics_double.h"

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

// TODO
// TODO
// TODO
// TODO Why do we rate clusters that in other alg are NA?
// TODO
// TODO
// TODO

Ranking::Ranking(std::vector<size_type>& goi, Clustering& clustering)
:	genes_of_interest(goi), clustering(clustering), rankings(clustering.get_source().get_gene_correlations().size1(), -99.0) // TODO use NaN instead
{
	rank_genes();
	rank_self();

	std::vector<pair<double, string>> results;
	for (int i=0; i<rankings.size(); i++) {
		results.push_back(make_pair(rankings(i), clustering.get_source().get_gene_name(i)));
	}
	sort(results.rbegin(), results.rend());
	for (auto r : results) {
		cout << r.second << " " << r.first << endl;
	}
}

// TODO define NDEBUG on release
void Ranking::rank_genes() {
	auto& gene_expression = clustering.get_source();
	gene_expression.debug();
	auto& gene_correlations = gene_expression.get_gene_correlations();
	bool meh = false;
	for (auto& cluster : clustering.get_clusters()) {
		cout << "c" << cluster.get_name() << endl;
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
		cout << "y" << endl;

		// compute rankings
		auto sub_matrix = project(gene_correlations, candidates, interesting_genes);
		if (meh) {
		cout << candidates(0) << " " << candidates(1) << endl;
		cout << interesting_genes.size() << endl;
		cout << sub_matrix << endl;
		cout << "------" << endl;
		}
		auto goi_count = interesting_genes.size();
		auto sub_rankings = project(rankings, candidates);
		noalias(sub_rankings) = prod(sub_matrix, ublas::scalar_vector<double>(goi_count)) / goi_count; // TODO there's some business of covariance and stuff that need be applied to results here
		if (meh) {
		cout << sub_rankings << endl;
		throw runtime_error("sup");// TODO dbg
		}
		//meh = true;

		// normalise scores within this cluster (TODO this implementation may be numerically unsound )
		// Note: it's different from R's output, either it's inaccurate or it's more accurate TODO (prolly the former; try gsl)
		/*auto mean = ublas::inner_prod(sub_rankings, ublas::scalar_vector<double>(sub_rankings.size())) / sub_rankings.size();
		cout << mean << endl;
		sub_rankings = sub_rankings - ublas::scalar_vector<double>(sub_rankings.size(), mean);
		auto standard_deviation = ublas::norm_2(sub_rankings) / sqrt(sub_rankings.size()); // TODO could we pass the iterator to gsl_stats_sd and such?. If not at least use gsl_*sqrt
		//sub_rankings = sub_rankings / standard_deviation;*/
		std::vector<double> sub_ranks(sub_rankings.begin(), sub_rankings.end()); // TODO really no way around this copy?
		double mean_ = gsl_stats_mean(sub_ranks.data(), 1, sub_ranks.size());
		//cout << mean_ << endl;
		//cout << standard_deviation << endl;
		double standard_deviation_ = gsl_stats_sd_m(sub_ranks.data(), 1, sub_ranks.size(), mean_);
		//cout << standard_deviation_ << endl;
		sub_rankings = (sub_rankings - ublas::scalar_vector<double>(sub_rankings.size(), mean_)) / standard_deviation_;
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
