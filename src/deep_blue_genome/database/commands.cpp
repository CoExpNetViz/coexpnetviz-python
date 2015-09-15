/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <deep_blue_genome/database/stdafx.h>
#include "commands.h"
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/writer/OrthologGroup.h>
#include <deep_blue_genome/common/reader/Database.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using namespace DEEP_BLUE_GENOME::COMMON::WRITER;
using namespace DEEP_BLUE_GENOME::COMMON::READER;

namespace DEEP_BLUE_GENOME {
namespace DATABASE {

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
	read_orthologs_yaml(database, config["orthologs"], data_root);

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

		importer.add_clustering(clustering_name, clustering_path);
	}

	database.save();
}
} // end impl namespace

void database_dump(const string& database_path, const std::string& dump_path) {
	Database database(database_path);

	YAML::Node root;

	// TODO
	// Gene collections
	/*for (auto gene_collection_node : config["gene_collections"]) {
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
	}*/

	// Orthologs
	for (auto&& group : database.get_ortholog_groups()) {
		root["ortholog_groups"].push_back(write_yaml_with_genes(group));
	}

	// TODO
	// Expression matrices
	/*for (auto matrix_node : config["expression_matrices"]) {
		string matrix_name = matrix_node["name"].as<string>();
		string matrix_path = prepend_path(data_root, matrix_node["path"].as<string>());

		importer.add_gene_expression_matrix(matrix_name, matrix_path);
	}

	// Clusterings
	for (auto clustering_node : config["clusterings"]) {
		string clustering_name = clustering_node["name"].as<string>();
		string clustering_path = prepend_path(data_root, clustering_node["path"].as<string>());

		importer.add_clustering(clustering_name, clustering_path, clustering_node["expression_matrix"].as<string>(""));
	}*/

	// Write to file
	ofstream out(dump_path);
	out.exceptions(ofstream::failbit | ofstream::badbit);
	out << YAML::Dump(root);
}

void database_create(const string& database_path, const string& yaml_path) {
	impl::database_update(true, database_path, yaml_path);
}

void database_add(const string& database_path, const string& yaml_path) {
	impl::database_update(false, database_path, yaml_path);
}

void database_verify(const std::string& database_path) {
	Database database(database_path);
	database.verify();
}

}} // end namespace
