#include <iostream>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/storage.hpp>

#include <string>
#include <vector>
#include <map>
#include <utility>

typedef boost::numeric::ublas::matrix<double> matrix;
typedef boost::numeric::ublas::matrix_indirect<matrix> matrix_indirect;
typedef matrix::size_type size_type;
//typedef boost::numeric::ublas::matrix<double>::array_type array_type;
typedef boost::numeric::ublas::indirect_array<> indirect_array;

class Gene
{
public:
	size_type index; // the index the gene has in Gene x T matrices
	std::string name;
};

class GenesOfInterest
{
public:
	std::vector<Gene*>& get_genes();

private:
	std::string name;
	std::vector<Gene*> genes;
};

// TODO fiddle with allocation types of matrices for performance reasons (maybe orientation type helps too) -> bounded array prolly best
class GeneExpression
{
public:
	/**
	 * Get correlation matrix of genes (rows) and genes of interest (columns)
	 */
	matrix& get_gene_correlations(); // TODO only fill the correlation columns corresponding to a gene of interest

private:
	std::string name; // name of dataset

	//std::map<Gene*, row> gene_mappings; // Note: it's faster to have it stored directly on the gene object (though then you'd have to look up the correct GeneExpression)
	//matrix<Gene, Expression> of double, expression_matrix;

	//std::map<Gene*, row> gene_of_interest_mappings;
	//matrix<Gene, Gene of interest> of double, gene_correlations;
};

class Cluster
{
public:
	/**
	 * Returns pair of (genes_of_interest, other_genes) in the cluster
	 */
	std::pair<indirect_array, indirect_array> get_genes(GenesOfInterest&) const;

private:
	std::map<GenesOfInterest*, std::pair<std::vector<Gene*>, std::vector<Gene*>>> goi_genes; // TODO instead of maps you could use vectors and add an index field to GenesOfInterest
};

class Clustering
{
public:
	const std::vector<Cluster>& get_clusters();
	GeneExpression& get_source();

private:
	std::string name;
	std::vector<Cluster> clusters;
	GeneExpression* source; // gene expression data we clustered
};

/**
 * Note: A negative ranking value for a gene means it wasn't ranked
 */
class Ranking
{
public:
	Ranking(GenesOfInterest&, Clustering&);

private:
	void rank_genes();
	void rank_self();

private:
	GenesOfInterest& genes_of_interest;
	Clustering& clustering;
	boost::numeric::ublas::vector<double> rankings; // size = genes.size(), gene_index -> ranking
};

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

int main() {
	// TODO load stuff
	//vector<GeneExpression> gene_expression_sets;

	// TODO actual calc
	std::vector<GenesOfInterest> genes_of_interest_sets;
	std::vector<Clustering> clusterings;

	map<GenesOfInterest*, std::vector<Ranking>> result_sets; // name of genes of interest set -> its rankings
	//results.reserve(clusterings.size());

	for (auto& clustering : clusterings) {
		for (auto& genes_of_interest : genes_of_interest_sets) {
			Ranking ranking(genes_of_interest, clustering);
			//results.emplace_back(ranking);
		}
	}

	// TODO print results

	return 0;
}
