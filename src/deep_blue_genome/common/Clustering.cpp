// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"
#include <deep_blue_genome/common/util.h>
#include <boost/spirit/include/qi.hpp>
#include <boost/function_output_iterator.hpp>
#include <utility>
#include <deep_blue_genome/common/TabGrammarRules.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

Clustering::Clustering(string name)
:	name(name)
{
}

Clustering::Clustering(string name, string path)
:	name(name)
{
	std::vector<string> genes;

	// Load
	read_file(path, [this, &genes](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		std::unordered_map<std::string, Cluster> clusters;
		size_type genes_missing = 0;

		auto on_cluster_item = [this, &clusters, &genes, &genes_missing](const std::vector<std::string>& line) {
			auto gene_name = line.at(0);
			// TODO currently assuming gene is already in canonical format
			auto cluster_id = line.at(1);
			auto it = clusters.find(cluster_id);
			if (it == clusters.end()) {
				it = clusters.emplace(piecewise_construct, make_tuple(cluster_id), make_tuple(cluster_id)).first;
			}
			auto& cluster = it->second;
			ensure(!contains(cluster, gene_name),
					(make_string() << "Clustering adds same gene to cluster twice: gene=" << gene_name <<
							", cluster=" << cluster_id).str(),
					ErrorType::GENERIC);
			cluster.add(gene_name);
			genes.emplace_back(gene_name);
		};

		TabGrammarRules rules;
		parse(begin, end, rules.line[on_cluster_item] % eol);

		// Move clusters' values to this->clusters
		this->clusters.reserve(clusters.size());
		for(auto& p : clusters) {
			this->clusters.emplace_back(std::move(p.second));
		}

		return begin;
	});
}

Clustering::const_iterator Clustering::begin() const {
	return clusters.begin();
}

Clustering::const_iterator Clustering::end() const {
	return clusters.end();
}

}
