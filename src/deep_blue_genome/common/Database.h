// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Species.h>

namespace DEEP_BLUE_GENOME {

class GeneExpressionMatrix;
class GeneExpressionMatrixClustering;
class Clustering;
class GeneMapping;
class GeneDescriptions;

/**
 * Deep Blue Genome Database
 *
 * Currently:
 * - loading of data sets in general (clusterings, expression matrices, ...)
 * - no querying, no transactions, no ...
 *
 * Invariant: No more than a single copy of data will be in memory.
 * (E.g. the same matrix won't be held in memory twice by Database.
 *  E.g. calling get_gene_descriptions twice in a row will return the same shared_ptr)
 *
 * When data is added to the database, it is validated, and then stored as read-only data.
 */
class Database {
public:
	/**
	 * @param database_path location of database. Currently this is a config with paths to all db files
	 */
	Database(std::string database_path);

	/**
	 * Update database with new data
	 *
	 * TODO specify expected yaml format
	 */
	void update(std::string yaml_path);

	/**
	 * @param species Name of species the matrix belongs to
	 * @param name Name of matrix
	 */
	std::shared_ptr<GeneExpressionMatrix> get_gene_expression_matrix(std::string species, std::string name);

	std::shared_ptr<GeneExpressionMatrixClustering> get_gene_expression_matrix_clustering(std::shared_ptr<GeneExpressionMatrix>, std::string clustering_name);

	/**
	 * @param species Name of species
	 */
	std::shared_ptr<GeneMapping> get_gene_mapping(std::string species);

	/**
	 * @param species Name of species
	 */
	std::shared_ptr<GeneDescriptions> get_gene_descriptions(std::string species);

	/**
	 * @param species Name of species
	 */
	std::shared_ptr<Species> get_species(std::string species);

private:
	/**
	 * Load instance of given Type.
	 *
	 * Ensures not having multiple instances of (Type, path) in memory;
	 * as long as you don't use multiple constructors of a single Type.
	 */
	template <class Type, class... Args>
	std::shared_ptr<Type> load(std::string path, Args... args);

	/**
	 * Get path to directory with all species info
	 */
	std::string get_species_dir(std::string species_name);

	/**
	 * Get path to file with general info on species
	 */
	std::string get_species_info_path(std::string species_name);

	std::string get_gene_expression_matrix_dir(std::string species, std::string name);
	std::string get_gene_expression_matrix_path(std::string species, std::string name);
	std::string get_gene_mapping_path(std::string species);
	std::string get_gene_descriptions_path(std::string species);
	std::string get_gene_expression_matrix_clustering_path(std::string species, std::string matrix_name, std::string name);

	/**
	 * Load object saved with save_to_binary
	 */
	template <class T>
	void load_from_binary(std::string path, T& object);

	template <class T>
	void save_to_binary(std::string path, const T& object);

private:
	std::string database_path;
	std::unordered_map<std::string, std::shared_ptr<Species>> species_infos;
};

} // end namespace


///////////////////////////////
// hpp

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>

namespace DEEP_BLUE_GENOME {

template <class Type, class... Args>
std::shared_ptr<Type> Database::load(std::string path, Args... args) {
	using namespace std;
	static unordered_map<std::string, weak_ptr<Type>> instances;

	// find instance
	auto it = instances.find(path);
	if (it == instances.end()) {
		it = instances.emplace(piecewise_construct, forward_as_tuple(path), forward_as_tuple()).first;
	}
	auto& instance = it->second;

	// get existing shared ptr
	if (auto ptr = instance.lock()) {
		return ptr;
	}

	// or make new shared ptr
	auto ptr = make_shared<Type>(args...);
	instance = ptr;
	load_from_binary(path, *ptr);
	return ptr;
}

template <class T>
void Database::load_from_binary(std::string path, T& object) {
	using namespace std;
	using namespace boost::iostreams;
	stream_buffer<file_source> buffer(path, ios::binary); // Note: this turned out to be strangely slightly faster than mapped_file
	boost::archive::binary_iarchive ar(buffer);
	ar >> object;
}

template <class T>
void Database::save_to_binary(std::string path, const T& object) {
	using namespace std;
	using namespace boost::iostreams;
	stream_buffer<file_sink> out(path, ios::binary);
	boost::archive::binary_oarchive ar(out);
	try {
		ar << object;
	}
	catch (const exception& e) {
		throw runtime_error("Error while writing to " + path + ": " + exception_what(e));
	}
}


}
