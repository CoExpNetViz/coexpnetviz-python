// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"
#include "util.h"

using namespace std;

Clustering::Clustering(string path, GeneExpression& gene_expression_)
:	name(path), gene_expression(gene_expression_)
{
	// Load
	read_file(path, [this](ifstream& in) {
		while (in.good()) {
			string gene_name;
			int cluster_id;
			in >> gene_name >> cluster_id;
			if (in.fail()) {
				break;
			}
			auto it = cluster_map.find(cluster_id);
			if (it == cluster_map.end()) {
				clusters.emplace_back();
				it = cluster_map.emplace(cluster_id, &clusters.back()).first;
			}
			it->second->add(gene_expression.get_gene_index(gene_name));
		}
	});
}

const std::vector<Cluster>& Clustering::get_clusters() const {
	return clusters;
}

GeneExpression& Clustering::get_source() const {
	return gene_expression;
}

indirect_array Clustering::get_genes() const {
	throw runtime_error("nope");
	// TODO
	/*indirect_array indices;
	for (auto& cluster : get_clusters()) {
		indices.insert(cluster.get_genes());
	}
	return indices;*/
}
