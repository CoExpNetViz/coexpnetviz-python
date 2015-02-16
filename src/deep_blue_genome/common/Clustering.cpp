// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/TabGrammarRules.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneCollection.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

// TODO did we drop variants along the way as we read in clusterings? If so, should we?
Clustering::Clustering(const std::string& name, const std::string& path, const std::string& expression_matrix, Database& database)
:	id(0), expression_matrix_id(0), name(name), database(database)
{
	// Load
	read_file(path, [this](const char* begin, const char* end) {
		using namespace boost::spirit::qi;

		std::unordered_map<std::string, Cluster> clusters;
		size_type genes_missing = 0;
		shared_ptr<GeneCollection> gene_collection;

		auto on_cluster_item = [this, &clusters, &genes_missing, &gene_collection](const std::vector<std::string>& line) {
			auto name = line.at(0);
			if (!gene_collection) {
				gene_collection_id = this->database.get_gene_variant(name).get_gene().get_gene_collection_id();
				gene_collection = this->database.get_gene_collection(gene_collection_id);
			}
			auto gene_id = gene_collection->get_gene_variant(name).get_gene().get_id();

			auto cluster_name = line.at(1);
			auto it = clusters.find(cluster_name);
			if (it == clusters.end()) {
				it = clusters.emplace(piecewise_construct, make_tuple(cluster_name), make_tuple(cluster_name)).first;
			}
			auto& cluster = it->second;
			ensure(!contains(cluster, gene_id),
					(make_string() << "Clustering adds same gene to cluster twice: gene=" << gene_id <<
							", cluster=" << cluster_name).str(),
					ErrorType::GENERIC);
			cluster.add(gene_id);
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

	if (expression_matrix != "") {
		expression_matrix_id = database.get_gene_expression_matrix_id(gene_collection_id, expression_matrix);
	}
}

Clustering::const_iterator Clustering::begin() const {
	return clusters.begin();
}

Clustering::const_iterator Clustering::end() const {
	return clusters.end();
}

void Clustering::database_insert() {
	assert(id == 0);

	// Insert clustering
	auto query = database.prepare("INSERT INTO clustering(name, gene_collection_id, expression_matrix_id) VALUES (%0q, %1q, %2q)");
	query.parse();
	auto matrix_id = expression_matrix_id == 0 ? mysqlpp::null : mysqlpp::Null<ExpressionMatrixId>(expression_matrix_id);
	auto result = query.execute(name, gene_collection_id, matrix_id);
	id = result.insert_id();

	// Insert clusters
	query = database.prepare("INSERT INTO cluster(clustering_id, name) VALUES (%0q, %1q)");
	query.parse();
	for (auto& cluster : clusters) {
		auto result = query.execute(id, cluster.get_name());
		ClusterId cluster_id = result.insert_id(); // TODO set cluster.id = id

		// Insert cluster items
		auto query = database.prepare("INSERT INTO cluster_item(cluster_id, gene_id) VALUES (%0q, %1q)");
		query.parse();
		for (auto gene_id : cluster) {
			query.execute(cluster_id, gene_id);
		}
	}
}

}
