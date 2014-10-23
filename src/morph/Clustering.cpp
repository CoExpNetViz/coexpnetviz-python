// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"
#include "util.h"

using namespace std;

Clustering::Clustering(string path, GeneExpression& gene_expression_)
:	name(path), gene_expression(gene_expression_)
{
	// Load
	std::map<string, Cluster*> cluster_map;
	read_file(path, [this, &cluster_map](ifstream& in) {
		while (in.good()) {
			string gene_name;
			string cluster_id;
			in >> gene_name >> cluster_id;
			if (in.fail()) {
				break;
			}
			auto it = cluster_map.find(cluster_id);
			if (it == cluster_map.end()) {
				clusters.emplace_back();
				it = cluster_map.emplace(cluster_id, &clusters.back()).first;
			}
			auto index = gene_expression.get_gene_index(gene_name);
			it->second->add(index);
			genes.emplace(index);
		}
	});
}

const std::vector<Cluster>& Clustering::get_clusters() const {
	return clusters;
}

GeneExpression& Clustering::get_source() const {
	return gene_expression;
}

const std::unordered_set<size_type>& Clustering::get_genes() const {
	return genes;
}
