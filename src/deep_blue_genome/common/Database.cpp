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
:	connection("db_tidie_deep_blue_genome", "127.0.0.1", "tidie", "4Ku8pxArMFzdS5Kt") // TODO don't hardcode username,password. Grab new password once this is no longer in here, psbsql05
//:	connection("db_tidie_deep_blue_genome", "127.0.0.1:55000", "tidie", "4Ku8pxArMFzdS5Kt") // TODO don't hardcode username,password. Grab new password once this is no longer in here, psbsql05
{
	storage_path = "/home/limyreth/dbg_db"; // TODO grab from settings table instead of hardcode

	// TODO load gene_collections, but careful with update further on
}

void Database::execute(const std::string& query) {
	ensure(connection.connected(), connection.error(), ErrorType::GENERIC); // it turned out mysql++ doesn't check this...
	connection.query(query).execute();
}

Query Database::prepare(const std::string& query) {
	ensure(connection.connected(), connection.error(), ErrorType::GENERIC);
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
	//execute("DELETE FROM gene_mapping");
	//execute("DELETE FROM gene_variant");
	//execute("DELETE FROM gene");
	//execute("DELETE FROM gene_parser_rule");
	//execute("DELETE FROM gene_collection");

	// Gene collections
	// TODO allow removal/overwrite of all sorts of things
	// TODO allow update of some things
	/*for (auto gene_collection_node : config["gene_collections"]) {
		// TODO when specified, it overwrites the previous definition of the gene collection. Do warn though. Also, makes it other than an update, since it kinda removes parts
		NullableGeneWebPage gene_web_page = gene_collection_node["gene_web_page"] ? NullableGeneWebPage(gene_collection_node["gene_web_page"].as<std::string>()) : mysqlpp::null;
		auto gene_collection = make_shared<GeneCollection>(
				gene_collection_node["name"].as<string>(), gene_collection_node["species"].as<string>(),
				gene_collection_node["gene_parser"], gene_web_page, *this
		);
		gene_collection->database_insert();
		gene_collections.emplace(gene_collection->get_id(), gene_collection);
	}*/

	// Gene mappings
	/*for (auto node : config["gene_mappings"]) {
		auto path = prepend_path(data_root, node.as<string>());
		cout << "Loading gene mapping '" << path << "'\n";
		DatabaseFileImport::add_gene_mappings(path, *this);
	}*/

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

GeneVariant Database::get_gene_variant(const std::string& name) {
	GeneVariant gene;
	for (auto& p : gene_collections) {
		if (p.second->try_get_gene_variant(name, gene)) {
			return gene;
		}
	}
	throw NotFoundException("Gene not part of a known gene collection: " + name);
}

Gene Database::get_gene(GeneId id) {
	auto query = prepare("SELECT gene_collection_id, name, ortholog_group_id FROM gene WHERE id = %0q");
	query.parse();
	auto result = query.store(id);

	if (result.num_rows() == 0) {
		throw NotFoundException("Gene id: " + id);
	}

	assert(result.num_rows() == 1);
	auto row = *result.begin();
	return Gene(id, row[0].conv<GeneCollectionId>(0), row[1].conv<std::string>(""), row[2].conv<NullableOrthologGroupId>(mysqlpp::null)); // Note: the arg to conv is ignored (you can check the source if paranoid)
}

GeneCollectionId Database::get_gene_collection_id(const std::string& name) {
	auto query = prepare("SELECT id FROM gene_collection WHERE LOWER(name) = LOWER(%0q)");
	query.parse();
	auto result = query.store(name);

	if (result.num_rows() == 0) {
		throw NotFoundException("Gene collection with name '" + name + "' not found");
	}

	assert(result.num_rows() == 1);
	auto row = *result.begin();
	return row[0];
}

ExpressionMatrixId Database::get_gene_expression_matrix_id(GeneCollectionId gene_collection_id, const std::string& name) {
	auto query = prepare("SELECT id FROM expression_matrix WHERE gene_collection_id = %0q AND name = %1q");
	query.parse();
	auto result = query.store(gene_collection_id, name);

	if (result.num_rows() == 0) {
		throw NotFoundException("Gene expression matrix with name '" + name + "' not found");
	}

	assert(result.num_rows() == 1);
	auto row = *result.begin();
	return row[0];
}

std::shared_ptr<GeneExpressionMatrix> Database::get_gene_expression_matrix(ExpressionMatrixId id) {
	return load<GeneExpressionMatrix>(id);
}

std::shared_ptr<GeneCollection> Database::get_gene_collection(GeneCollectionId id) {
	return load<GeneCollection>(id);
}

std::string Database::get_gene_expression_matrix_values_file(GeneExpressionMatrixId id) {
	return (make_string() << storage_path << "/gene_expression_matrices/" << id << "/matrix").str();
}

GeneVariantId Database::get_gene_variant_id(GeneId gene, NullableSpliceVariantId splice_variant) {
	auto query = prepare("SELECT id FROM gene_variant WHERE gene_id = %0q AND splice_variant_id = %1q");
	query.parse();
	auto result = query.store(gene, splice_variant);

	if (result.num_rows() == 0) {
		throw NotFoundException((make_string() << "Failed to find splice variant " << splice_variant << " of gene id " << gene).str());
	}

	assert(result.num_rows() == 1);
	auto row = *result.begin();
	return row[0];
}

GeneVariant Database::get_gene_variant(GeneVariantId id) {
	return GeneVariant(id, *this);
}

} // end namespace
