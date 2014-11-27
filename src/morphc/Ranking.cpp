// Author: Tim Diels <timdiels.m@gmail.com>

#include "Ranking.h"
#include "util.h"
#include "gsl/gsl_statistics_double.h"
#include <fstream>
#include <iomanip>
#include <cmath>

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

namespace MORPHC {

size_type K = 1000;

Ranking::Ranking(const std::vector<size_type>& goi, Clustering& clustering, std::string name)
:	genes_of_interest(goi), clustering(clustering), rankings(clustering.get_source().get_gene_correlations().size1(), nan("undefined")), ausr(-1.0)
{
	rank_genes(goi, rankings);
	rank_self();
}

void Ranking::rank_genes(const std::vector<size_type>& genes_of_interest, boost::numeric::ublas::vector<double>& rankings) {
	auto& gene_expression = clustering.get_source();
	auto& gene_correlations = gene_expression.get_gene_correlations();
	for (auto& cluster : clustering.get_clusters()) {
		auto& cluster_genes = cluster.get_genes();

		// interesting_genes array
		MORPHC::array interesting_genes_(genes_of_interest.size());
		auto in_cluster = [&cluster_genes](size_type gene) {
			return contains(cluster_genes, gene);
		};
		auto it = copy_if(genes_of_interest.begin(), genes_of_interest.end(), interesting_genes_.begin(), in_cluster);
		MORPHC::indirect_array interesting_genes(distance(interesting_genes_.begin(), it), interesting_genes_);
		if (interesting_genes.size() == 0)
			continue;

		// candidate genes array
		MORPHC::array candidates_(cluster_genes.size());
		auto is_not_gene_of_interest = [&genes_of_interest](size_type gene) {
			return !contains(genes_of_interest, gene);
		};
		auto it2 = copy_if(cluster_genes.begin(), cluster_genes.end(), candidates_.begin(), is_not_gene_of_interest);
		MORPHC::indirect_array candidates(distance(candidates_.begin(), it2), candidates_);
		if (candidates.size() == 0)
			continue;

		// compute rankings
		auto sub_matrix = project(gene_correlations, candidates, interesting_genes);
		auto goi_count = interesting_genes.size();
		auto sub_rankings = project(rankings, candidates);
		noalias(sub_rankings) = prod(sub_matrix, ublas::scalar_vector<double>(goi_count)) / goi_count;

		// normalise scores within this cluster (TODO this implementation may be numerically unsound )
		// Note: it's different from R's output, either it's inaccurate or it's more accurate TODO (prolly the former; try gsl)
		/*auto mean = ublas::inner_prod(sub_rankings, ublas::scalar_vector<double>(sub_rankings.size())) / sub_rankings.size();
		cout << mean << endl;
		sub_rankings = sub_rankings - ublas::scalar_vector<double>(sub_rankings.size(), mean);
		auto standard_deviation = ublas::norm_2(sub_rankings) / sqrt(sub_rankings.size()-1); // TODO could we pass the iterator to gsl_stats_sd and such?. If not at least use gsl_*sqrt
		//sub_rankings = sub_rankings / standard_deviation;*/
		std::vector<double> sub_ranks(sub_rankings.begin(), sub_rankings.end()); // TODO really no way around this copy? could like... use ublas. inspect gsl source for correctness
		double mean_ = gsl_stats_mean(sub_ranks.data(), 1, sub_ranks.size());
		//cout << mean_ << endl;
		//cout << standard_deviation << endl;
		double standard_deviation_ = gsl_stats_sd_m(sub_ranks.data(), 1, sub_ranks.size(), mean_);
		//cout << standard_deviation_ << endl;
		sub_rankings = (sub_rankings - ublas::scalar_vector<double>(sub_rankings.size(), mean_)) / standard_deviation_;
	}
}

void Ranking::rank_self() {
	// find rank_indices of leaving out a gene of interest one by one
	std::vector<size_type> rank_indices;
	boost::numeric::ublas::vector<double> rankings;
	for (auto gene : genes_of_interest) {
		// TODO could also copy original rankings and recalc only the cluster of the gene we removed
		rankings = ublas::vector<double>(this->rankings.size(), nan("undefined"));
		auto goi = genes_of_interest;
		goi.erase(find(goi.begin(), goi.end(), gene));
		rank_genes(goi, rankings);
		double rank = rankings(gene);
		if (std::isnan(rank)) {
			// gene undetected, give penalty
			rank_indices.emplace_back(2*K-1);
		}
		else {
			size_type count = count_if(rankings.begin(), rankings.end(), [rank](double val){return val > rank;});
			rank_indices.emplace_back(count);
		}
	}
	sort(rank_indices.begin(), rank_indices.end());

	// calculate ausr
	double auc = 0.0; // area under curve
	for (size_type i = 0; i < K; i++) {
		// TODO can continue last search for upper bound where we left last one...
		auto supremum = upper_bound(rank_indices.begin(), rank_indices.end(), i);
		auto count = distance(rank_indices.begin(), supremum);
		auc += (double)count / rank_indices.size();
	}
	ausr = auc / K;
}

void Ranking::save(std::string path, int top_k) {
	// sort results
	std::vector<pair<double, string>> results;
	for (int i=0; i<rankings.size(); i++) {
		results.push_back(make_pair(rankings(i), clustering.get_source().get_gene_name(i)));
	}
	sort(results.rbegin(), results.rend());

	// output results
	ofstream out(path + "/" + name);
	out << setprecision(9) << scientific;
	out << "AUSR: " << ausr << "\n\n"; // Note: "\n" is faster than std::endl
	for (int i=0; i<results.size() && i<top_k; i++) {
		auto& r = results.at(i);
		if (std::isnan(r.first))
			break;
		out << r.second << " " << r.first << "\n";
	}
}

bool Ranking::operator>(const Ranking& other) const {
	return ausr > other.ausr;
}

}
