// Author: Tim Diels <timdiels.m@gmail.com>

#include "Database.h"
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/common/GeneDescriptions.h>
#include <deep_blue_genome/common/GeneMapping.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/GeneExpressionMatrixClustering.h>
#include <deep_blue_genome/common/Clustering.h>

using namespace std;

// TODO const string& everywhere
namespace DEEP_BLUE_GENOME {

Database::Database(std::string database_path)
:	database_path(database_path)
{
	if (boost::filesystem::exists(get_database_info_path())) {
		load_from_binary(get_database_info_path(), *this);
	}
	cout << (uint64_t)this << endl;
}

void Database::update(std::string yaml_path) {
	// TODO protect from concurrent updates (by locking entire db on update); probably should disable reading in other instances while locked
	return;
	YAML::Node config = YAML::LoadFile(yaml_path);
	string data_root = config["species_data_path"].as<string>(".");

	for (auto species_node : config["species"]) {
		auto species__ = make_shared<Species>(species_node["name"].as<string>(), *this);
		species_infos.emplace(species_node["name"].as<string>(), species__); // TODO before emplacing first try to find an existing one
		auto& species_ = *species__;
		species_names.emplace_back(species_.get_name());

		string species_root = prepend_path(data_root, species_node["data_path"].as<string>("."));

		species_.set_gene_pattern(species_node["gene_pattern"].as<string>(""));

		// TODO allow removal (null in yaml?)
		if (species_node["gene_web_page"]) {
			species_.set_gene_web_page(species_node["gene_web_page"].as<string>());
		}

		// TODO allow removal (e.g. set it to null in yaml file)
		if (species_node["gene_descriptions"]) {
			cout << "Loading gene descriptions of species '" << species_.get_name() << "'\n";
			species_.has_gene_descriptions(true);
			string path = prepend_path(species_root, species_node["gene_descriptions"].as<string>());
			GeneDescriptions descriptions(path);
			save_to_binary(get_gene_descriptions_path(species_.get_name()), descriptions);
		}

		// TODO allow removal (e.g. set it to null in yaml file)
		if (species_node["gene_mapping"]) {
			cout << "Loading gene mapping of species '" << species_.get_name() << "'\n";
			species_.has_gene_mapping(true);
			string path = prepend_path(species_root, species_node["gene_mapping"].as<string>());
			GeneMapping mapping(path);
			save_to_binary(get_gene_mapping_path(species_.get_name()), mapping);
		}

		for (auto matrix_node : species_node["expression_matrices"]) {
			string matrix_name = matrix_node["name"].as<string>();
			string matrix_path = prepend_path(species_root, matrix_node["path"].as<string>());

			cout << "Loading gene expression matrix '" << matrix_name << "' of species '" << species_.get_name() << "'\n";
			auto matrix = make_shared<GeneExpressionMatrix>(matrix_name, species_.get_name(), matrix_path, *this);
			save_to_binary(get_gene_expression_matrix_path(species_.get_name(), matrix_name), *matrix);

			vector<string> clusterings;
			for (auto clustering_node : matrix_node["clusterings"]) {
				string clustering_name = clustering_node["name"].as<string>();
				string clustering_path = prepend_path(species_root, clustering_node["path"].as<string>());
				clusterings.emplace_back(clustering_name);

				cout << "Loading clustering '" << clustering_name << "' of gene expression matrix '" << matrix_name << "' of species '" << species_.get_name() << "'\n";
				Clustering clustering(clustering_name, clustering_path);
				GeneExpressionMatrixClustering gem_clustering(matrix, clustering);
				save_to_binary(get_gene_expression_matrix_clustering_path(species_.get_name(), matrix_name, clustering_name), gem_clustering);
			}

			species_.update_gene_expression_matrix(matrix_name, clusterings);
		}
	}

	save_to_binary(get_database_info_path(), *this);
	cout << "Saved updated database state" << endl;
}

std::string Database::get_database_info_path() {
	return database_path + "/main_info";
}

std::string Database::get_species_dir(std::string species_name) {
	return database_path + "/species/" + to_file_name(species_name);
}

std::string Database::get_gene_expression_matrix_dir(std::string species, std::string name) {
	return get_species_dir(species) + "/gene_expression_matrices/" + to_file_name(name);
}

std::string Database::get_gene_expression_matrix_path(std::string species, std::string name) {
	return  get_gene_expression_matrix_dir(species, name) + "/matrix";
}

std::string Database::get_gene_mapping_path(std::string species) {
	return get_species_dir(species) + "/gene_mapping";
}

std::string Database::get_gene_descriptions_path(std::string species) {
	return get_species_dir(species) + "/gene_descriptions";
}

std::string Database::get_gene_expression_matrix_clustering_path(std::string species, std::string matrix_name, std::string name) {
	return get_gene_expression_matrix_dir(species, matrix_name) + "/clusterings/" + to_file_name(name);
}

std::shared_ptr<GeneExpressionMatrix> Database::get_gene_expression_matrix(std::string species, std::string name) {
	return load<GeneExpressionMatrix>(get_gene_expression_matrix_path(species, name), name, species, *this);
}

std::shared_ptr<GeneExpressionMatrixClustering> Database::get_gene_expression_matrix_clustering(std::shared_ptr<GeneExpressionMatrix> matrix, std::string name) {
	return load<GeneExpressionMatrixClustering>(get_gene_expression_matrix_clustering_path(matrix->get_species_name(), matrix->get_name(), name), matrix, name);
}

std::shared_ptr<GeneMapping> Database::get_gene_mapping(std::string species) {
	return load<GeneMapping>(get_gene_mapping_path(species), species);
}

std::shared_ptr<GeneDescriptions> Database::get_gene_descriptions(std::string species) {
	return load<GeneDescriptions>(get_gene_descriptions_path(species), species);
}

std::shared_ptr<Species> Database::get_species(std::string species) {
	cout << "db" << database_path << endl;
	auto it = species_infos.find(species);
	assert(it != species_infos.end());
	return it->second;
}

Iterable<Database::name_iterator> Database::get_species_names() {
	return make_iterable(species_names.cbegin(), species_names.cend());
}



} // end namespace
