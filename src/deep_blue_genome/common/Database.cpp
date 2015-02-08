// Author: Tim Diels <timdiels.m@gmail.com>

#include "Database.h"
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/GeneCollection.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/Clustering.h>
#include <deep_blue_genome/common/DatabaseFileImport.h>

using namespace std;
using namespace mysqlpp;

// TODO const string& everywhere except in returns
namespace DEEP_BLUE_GENOME {

Database::Database()
:	connection("psbsql05", "db_tidie_deep_blue_genome", "tidie", "4Ku8pxArMFzdS5Kt") // TODO don't hardcode username,password. Grab new password once this is no longer in here
{
}

void Database::execute(const std::string& query) {
	connection.query(query).execute();
}

Query Database::prepare(const std::string& query) {
	return connection.query(query);
}

void Database::update(std::string yaml_path) {
	YAML::Node config = YAML::LoadFile(yaml_path);
	string data_root = config["species_data_path"].as<string>(".");

	// TODO don't clear first
	// for testing, clear db first
	execute("DELETE FROM cluster_item");
	execute("DELETE FROM cluster");
	execute("DELETE FROM clustering");
	execute("DELETE FROM expression_matrix_row");
	execute("DELETE FROM expression_matrix");
	execute("DELETE FROM gene_mapping");
	execute("DELETE FROM gene");
	execute("DELETE FROM gene_collection");
	execute("DELETE FROM genome");

	// Gene collections
	// TODO allow removal/overwrite of all sorts of things
	for (auto gene_collection_node : config["gene_collections"]) {
		// TODO when specified, it overwrites the previous definition of the gene collection. Do warn though. Also, makes it other than an update, since it kinda removes parts
		GeneCollection gene_collection(gene_collection_node["name"].as<string>(), gene_collection_node["gene_format_match"].as<string>(), gene_collection_node["gene_format_replace"].as<string>(), gene_collection_node["gene_web_page"].as<string>(""), *this);
		gene_collection.database_insert(*this); // TODO also update
	}

	// Gene mappings

	for (auto node : config["gene_mappings"]) {
		auto path = prepend_path(data_root, node.as<string>());
		cout << "Loading gene mapping '" << path << "'\n";
		DatabaseFileImport::add_gene_mappings(path, *this);
	}

	// Functional annotations
	for (auto node : config["functional_annotations"]) {
		auto path = prepend_path(data_root, node.as<string>());
		cout << "Loading gene descriptions '" << path << "'\n";

		DatabaseFileImport::add_functional_annotations(path, *this);
	}

	// Orthologs
	for (auto node : config["orthologs"]) { // TODO use some orthologs in our debug config will ya
		auto path = prepend_path(data_root, node.as<string>());
		cout << "Loading orthologs '" << path << "'\n";
		DatabaseFileImport::add_orthologs(path, *this);
	}

	// Expression matrices
	for (auto matrix_node : config["expression_matrices"]) {
		string matrix_name = matrix_node["name"].as<string>();
		string matrix_path = prepend_path(data_root, matrix_node["path"].as<string>());

		cout << "Loading gene expression matrix '" << matrix_name << "'\n";
		GeneExpressionMatrix matrix(matrix_name, matrix_path, *this);
		matrix.database_insert();
	}

	// Clusterings
	for (auto clustering_node : config["clusterings"]) {
		string clustering_name = clustering_node["name"].as<string>();
		string clustering_path = prepend_path(data_root, clustering_node["path"].as<string>());

		cout << "Loading clustering '" << clustering_name << "'\n";
		Clustering clustering(clustering_name, clustering_path, clustering_node["expression_matrix"].as<string>(""), *this);
		clustering.database_insert();
	}

	// TODO allow removal (--rm-gene-mappings --rm-functional-annotations --rm-expression-matrices --rm-orthologs --rm-clusterings --rm-gene-collections)
}

Gene Database::get_gene(const std::string& name) {
	Gene gene;
	for (auto& p : gene_collections) {
		if (p.second->try_get_gene_by_name(name, gene)) {
			return gene;
		}
	}
	throw NotFoundException("Gene not part of a known gene collection: " + name);
}


} // end namespace
