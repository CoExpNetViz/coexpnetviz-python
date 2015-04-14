// Author: Tim Diels <timdiels.m@gmail.com>

#include "commands.h"
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/common/database_all.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace DATABASE {

void database_create(const string& database_path, const string& yaml_path) {
	impl::database_update(true, database_path, yaml_path);
}

void database_add(const string& database_path, const string& yaml_path) {
	impl::database_update(false, database_path, yaml_path);
}

namespace impl {
void database_update(bool create, const string& database_path, const string& yaml_path) {
	Database database(database_path, create);

	YAML::Node config = YAML::LoadFile(yaml_path);
	string data_root = config["species_data_path"].as<string>(".");
	DataFileImport importer(database);

	// Gene collections
	for (auto gene_collection_node : config["gene_collections"]) {
		NullableGeneWebPage gene_web_page;
		if (gene_collection_node["gene_web_page"]) {
			gene_web_page = gene_collection_node["gene_web_page"].as<std::string>();
		}

		database.add(make_unique<GeneCollection>(
				database,
				gene_collection_node["name"].as<string>(), gene_collection_node["species"].as<string>(),
				gene_collection_node["gene_parser"], gene_web_page
		));
	}

	// Gene mappings
	for (auto node : config["gene_mappings"]) {
		auto path = prepend_path(data_root, node.as<string>());
		importer.add_gene_mappings(path);
	}

	// Functional annotations
	for (auto node : config["functional_annotations"]) {
		auto path = prepend_path(data_root, node.as<string>());
		importer.add_functional_annotations(path);
	}

	// Orthologs
	for (auto node : config["orthologs"]) {
		auto path = prepend_path(data_root, node["path"].as<string>());
		importer.add_orthologs(node["name"].as<string>(), path);
	}

	// Expression matrices
	for (auto matrix_node : config["expression_matrices"]) {
		string matrix_name = matrix_node["name"].as<string>();
		string matrix_path = prepend_path(data_root, matrix_node["path"].as<string>());

		importer.add_gene_expression_matrix(matrix_name, matrix_path);
	}

	// Clusterings
	for (auto clustering_node : config["clusterings"]) {
		string clustering_name = clustering_node["name"].as<string>();
		string clustering_path = prepend_path(data_root, clustering_node["path"].as<string>());

		importer.add_clustering(clustering_name, clustering_path, clustering_node["expression_matrix"].as<string>(""));
	}

	database.save();
}
} // end impl namespace

}} // end namespace
