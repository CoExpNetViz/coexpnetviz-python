// Author: Tim Diels <timdiels.m@gmail.com>

#include "Database.h"
#include <yaml-cpp/yaml.h>
#include <boost/filesystem.hpp>
#include <deep_blue_genome/common/GeneCollection.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/Clustering.h>
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/common/SpliceVariant.h>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/OrthologGroup.h>

using namespace std;

// TODO const string& everywhere except in returns
namespace DEEP_BLUE_GENOME {

Database::Database(std::string path)
:	database_path(std::move(path))
{
	auto main_file = get_main_file();
	if (boost::filesystem::exists(main_file)) {
		Serialization::load_from_binary(main_file, *this);
	}
}

void Database::clear() {
	ortholog_groups.clear();
	gene_collections.clear();
}

void Database::update(std::string yaml_path) {
	clear();

	// TODO allow removal/overwrite of all sorts of things
	// TODO allow update of some things

	YAML::Node config = YAML::LoadFile(yaml_path);
	string data_root = config["species_data_path"].as<string>(".");
	DataFileImport importer(*this);

	// Gene collections
	for (auto gene_collection_node : config["gene_collections"]) {
		// TODO when specified, it overwrites the previous definition of the gene collection. Do warn though. Also, makes it other than an update, since it kinda removes parts
		NullableGeneWebPage gene_web_page;
		if (gene_collection_node["gene_web_page"]) {
			gene_web_page = gene_collection_node["gene_web_page"].as<std::string>();
		}

		gene_collections.emplace_back(make_unique<GeneCollection>(
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
	for (auto node : config["orthologs"]) { // TODO use some orthologs in our debug config will ya
		auto path = prepend_path(data_root, node.as<string>());
		importer.add_orthologs(path);
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

	// TODO allow removal (--rm-gene-mappings --rm-functional-annotations --rm-expression-matrices --rm-orthologs --rm-clusterings --rm-gene-collections)

	Serialization::save_to_binary(get_main_file(), *this);
}

GeneVariant& Database::get_gene_variant(const std::string& name) {
	auto variant = try_get_gene_variant(name);
	if (variant)
		return *variant;
	else
		throw NotFoundException("Gene not part of a known gene collection: " + name);
}

GeneVariant* Database::try_get_gene_variant(const std::string& name) {
	for (auto& gene_collection : gene_collections) {
		auto variant = gene_collection->try_get_gene_variant(name);
		if (variant) {
			return variant;
		}
	}
	return nullptr;
}

OrthologGroup& Database::add_ortholog_group(std::string external_id) {
	// TODO no 2 ortholog groups should have the same external id
	ortholog_groups.emplace_back(make_unique<OrthologGroup>(std::move(external_id)));
	return *ortholog_groups.back();
}

GeneCollection& Database::get_gene_collection(const std::string& name) {
	return **find_if(gene_collections.begin(), gene_collections.end(), [name](unique_ptr<GeneCollection>& gene_collection) {
		return gene_collection->get_name() == name;
	});
}

void Database::erase(OrthologGroup& group) {
	auto it = find_if(ortholog_groups.begin(), ortholog_groups.end(), [&group](unique_ptr<OrthologGroup>& g) {
		return g.get() == &group;
	});
	assert(it != ortholog_groups.end());
	ortholog_groups.erase(it);
}

string Database::get_main_file() const {
	return database_path + "/db";
}

/*
 * Validate everything that constructors could not have checked (uniqueness, ...)
 * Validate:
 * - unique(GeneCollection.name)
 * Some of these can be ensured via invariants... But uniqueness checks are preferably only done after done loading all data (e.g. when adding in bulk to the database)
 */

} // end namespace
