// Author: Tim Diels <timdiels.m@gmail.com>

#include "Ranking.h"
#include "util.h"
#include "gsl/gsl_statistics_double.h"
#include <fstream>
#include <iomanip>
#include <cmath>
#include <boost/algorithm/string.hpp>

using namespace std;
namespace ublas = boost::numeric::ublas;
using namespace ublas;

namespace MORPHC {

size_type K = 1000;

Ranking_ClusterInfo::Ranking_ClusterInfo(const std::vector<size_type>& genes_of_interest, const Cluster& c)
{
	auto& cluster = const_cast<Cluster&>(c);
	auto is_goi = [&genes_of_interest](size_type gene) {
		return contains(genes_of_interest, gene);
	};
	auto candidates_begin = partition(cluster.begin(), cluster.end(), is_goi); // Note: modifying the order of cluster genes doesn't really change the cluster

	goi = MORPHC::indirect_array(&*cluster.begin(), &*candidates_begin); // Note: ublas indirect_array is making me do ugly things
	candidates = MORPHC::indirect_array(&*candidates_begin, &*cluster.end());
	genes = MORPHC::indirect_array(&*cluster.begin(), &*cluster.end());
}

Ranking::Ranking(std::vector<size_type> goi, std::shared_ptr<Clustering> clustering, std::string name)
:	genes_of_interest(goi), clustering(clustering), ausr(-1.0), name(name)
{
	Rankings rankings(clustering->get_source().get_gene_correlations().size1(), nan("undefined"));

	// fill rankings with intermediary values
	rank_genes(goi, rankings);

	// finish calculation of rankings
	final_rankings = Rankings(rankings.size(), nan("undefined"));
	finalise_ranking(rankings);

	// calculate ausr
	rank_self(rankings);
}

void Ranking::rank_genes(const std::vector<size_type>& genes_of_interest, Rankings& rankings) {
	for (auto& p : *clustering) {
		auto& cluster = p.second;

		auto& info = cluster_info.emplace(piecewise_construct, make_tuple(&cluster), make_tuple(genes_of_interest, cluster)).first->second;

		// skip if no goi or candidates in this cluster
		if (info.get_goi_count() == 0 || info.candidates.size() == 0)
			continue;

		// compute rankings
		auto sub_matrix = project(get_gene_correlations(), info.genes, info.goi);
		auto sub_rankings = project(rankings, info.genes);
		noalias(sub_rankings) = prod(sub_matrix, ublas::scalar_vector<double>(info.get_goi_count())); // TODO might want to consider more numerically stable kind of mean
	}
}

void Ranking::finalise_ranking(const Rankings& rankings) {
	for (auto& p : *clustering) {
		auto& info = cluster_info.at(&p.second);
		finalise_sub_ranking(rankings, final_rankings, info.candidates, info);
	}
}

void Ranking::finalise_sub_ranking(const Rankings& rankings, Rankings& final_rankings, const MORPHC::indirect_array& sub_indices, Ranking_ClusterInfo& info, long excluded_goi) {
	if (info.get_goi_count() == 0 || info.candidates.size() == 0) {
		return; // in this case, all values in these ranking will (and should) be NaN
	}

	auto sub_rankings = project(rankings, sub_indices);
	auto final_sub_rankings = project(final_rankings, sub_indices);
	if (excluded_goi > 0) {
		auto sub_matrix = project(column(get_gene_correlations(), excluded_goi), sub_indices);
		noalias(final_sub_rankings) = (sub_rankings - sub_matrix) / (info.get_goi_count() - 1);
	}
	else {
		noalias(final_sub_rankings) = sub_rankings / info.get_goi_count();
	}

	// unset partial ranking of goi genes in final ranking
	for (auto gene : info.goi) {
		if (gene != excluded_goi) {
			final_rankings(gene) = nan("undefined");
		}
	}

	// Normalise scores within this cluster: uses GSL => numerically stable
	std::vector<double> sub_ranks(final_sub_rankings.begin(), final_sub_rankings.end());
	double mean_ = gsl_stats_mean(sub_ranks.data(), 1, sub_ranks.size());
	double standard_deviation_ = gsl_stats_sd_m(sub_ranks.data(), 1, sub_ranks.size(), mean_);
	final_sub_rankings = (final_sub_rankings - ublas::scalar_vector<double>(final_sub_rankings.size(), mean_)) / standard_deviation_;

	// Normalise scores within this cluster: perhaps numerically unstable TODO
	/*auto mean = ublas::inner_prod(sub_rankings, ublas::scalar_vector<double>(sub_rankings.size())) / sub_rankings.size();
	sub_rankings = sub_rankings - ublas::scalar_vector<double>(sub_rankings.size(), mean);
	auto standard_deviation = ublas::norm_2(sub_rankings) / sqrt(sub_rankings.size()-1); // TODO could we pass the iterator to gsl_stats_sd and such?. If not at least use gsl_*sqrt
	//sub_rankings = sub_rankings / standard_deviation;*/
}

void Ranking::rank_self(const Rankings& rankings) {
	// find rank_indices of leaving out a gene of interest one by one
	std::vector<size_type> rank_indices;
	Rankings final_rankings = this->final_rankings;
	for (auto p : cluster_info) {
		auto& cluster = *p.first;
		auto& info = p.second;
		MORPHC::array candidates_and_gene_(info.candidates.size() + 1);
		copy(info.candidates.begin(), info.candidates.end(), candidates_and_gene_.begin());
		for (auto gene : info.goi) {
			candidates_and_gene_[candidates_and_gene_.size()-1] = gene;
			MORPHC::indirect_array candidates_and_gene(candidates_and_gene_.size(), candidates_and_gene_);
			finalise_sub_ranking(rankings, final_rankings, candidates_and_gene, info, gene);

			double rank = final_rankings(gene);
			if (std::isnan(rank)) {
				// gene undetected, give penalty
				rank_indices.emplace_back(2*K-1); // TODO shouldn't this be at least the amount of genes in the dataset? Or otherwise, shouldn't those that do appear be maxed out to 2K-1?
			}
			else {
				// TODO currently we do len(GOI) passes on the whole ranking, a sort + single pass is probably faster
				size_type count = count_if(final_rankings.begin(), final_rankings.end(), [rank](double val){return val > rank && !std::isnan(val);});
				rank_indices.emplace_back(count);
			}
		}
		project(final_rankings, info.genes) = project(this->final_rankings, info.genes); // restore what we changed
	}
	assert(rank_indices.size() == genes_of_interest.size());
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

void Ranking::save(std::string path, int top_k, const GeneDescriptions& descriptions, std::string gene_web_page_template) {
	// Sort results
	std::vector<pair<double, string>> results; // vec<(rank, gene)>
	auto& gene_expression = clustering->get_source();
	for (int i=0; i<final_rankings.size(); i++) {
		if (std::isnan(final_rankings(i)))
			continue; // don't include unranked genes in results
		results.push_back(make_pair(final_rankings(i), gene_expression.get_gene_name(i)));
	}
	sort(results.rbegin(), results.rend());

	// Out put results
	ofstream out(path + "/" + name);
	out.exceptions(ofstream::failbit | ofstream::badbit);
	out << setprecision(9) << fixed;
	out << "AUSR: " << ausr << "\n"; // Note: "\n" is faster to out put than std::endl
	out << setprecision(9) << scientific;
	out << "Gene expression data: " << clustering->get_source().get_name() << "\n";
	out << "Clustering: " << clustering->get_name() << "\n";
	out << "\n";
	for (int i=0; i<results.size() && i<top_k; i++) {
		auto& r = results.at(i);
		auto& gene = r.second;
		assert(!std::isnan(r.first));
		std::string web_page = gene_web_page_template;
		boost::replace_all(web_page, "$name", gene);
		out << gene << "\t" << r.first << "\t" << descriptions.get(gene) << "\t" << web_page << "\n";
	}
}

bool Ranking::operator>(const Ranking& other) const {
	return ausr > other.ausr;
}

double Ranking::get_ausr() const {
	return ausr;
}

const GeneCorrelations& Ranking::get_gene_correlations() {
	return clustering->get_source().get_gene_correlations();
}

}
