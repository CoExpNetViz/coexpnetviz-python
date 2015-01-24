// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneExpressionMatrixClustering.h"
#include <deep_blue_genome/common/util.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

GeneExpressionMatrixClustering::GeneExpressionMatrixClustering(std::shared_ptr<GeneExpressionMatrix> matrix, std::string name)
:	name(name), gene_expression_matrix(matrix)
{
}
GeneExpressionMatrixClustering::GeneExpressionMatrixClustering(std::shared_ptr<GeneExpressionMatrix> gene_expression_matrix, const Clustering& clustering)
:	name(clustering.get_name()), gene_expression_matrix(gene_expression_matrix)
{
	size_type genes_missing;
	vector<size_type> genes;

	// Convert gene names to indices
	for (auto& cluster : clustering) {
		clusters.emplace_back(cluster.get_name());
		auto& new_cluster = clusters.back();

		for (auto& gene : cluster) {
			if (!gene_expression_matrix->has_gene(gene)) {
				// Not all clusterings are generated from an expression matrix.
				// So a clustering can contain genes that are not present in the expression matrix.
				genes_missing++;
			}
			else {
				auto index = gene_expression_matrix->get_gene_index(gene);
				new_cluster.add(index);
				genes.emplace_back(index);
			}
		}
	}

	if (genes_missing > 0) {
		cerr << "Warning: " << genes_missing << " genes in clustering not present in expression matrix\n";
	}

	////////////////////////////////////
	// Group together unclustered genes
	// Note: genes contains indices of all genes found in a cluster so far, any missing in range [0, expression_matrix.rows), will be added as unclusterd
	clusters.emplace_back(" unclustered"); // the leading space is to avoid accidentally overwriting a cluster in the clustering file named 'unclustered'
	auto& cluster = clusters.back();

	sort(genes.begin(), genes.end());
	genes.emplace_back(gene_expression_matrix->get().size1());
	size_type last_gene = 0;

	for (auto gene : genes) {
		for (size_type i=last_gene+1; i<gene; i++) {
			cluster.add(i);
		}
		last_gene = gene;
	}

	if (cluster.empty()) {
		clusters.pop_back();
	}
}

GeneExpressionMatrixClustering::const_iterator GeneExpressionMatrixClustering::begin() const {
	return clusters.begin();
}

GeneExpressionMatrixClustering::const_iterator GeneExpressionMatrixClustering::end() const {
	return clusters.end();
}

GeneExpressionMatrix& GeneExpressionMatrixClustering::get_source() const {
	return *gene_expression_matrix;
}

std::string GeneExpressionMatrixClustering::get_name() const {
	return name;
}

}
