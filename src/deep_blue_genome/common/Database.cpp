// Author: Tim Diels <timdiels.m@gmail.com>

#include <deep_blue_genome/common/database_all.h>
#include <boost/filesystem.hpp>
#include <deep_blue_genome/common/Serialization.h>

using namespace std;

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
	ensure(it != ortholog_groups.end(),
			(make_string() << "Cannot erase ortholog group '" << group << "' as it does not exist").str(),
			ErrorType::GENERIC
	);
	ortholog_groups.erase(it);
}

string Database::get_main_file() const {
	return database_path + "/db";
}

void Database::add(std::unique_ptr<GeneCollection>&& gene_collection) {
	auto it = find_if(gene_collections.begin(), gene_collections.end(), [&gene_collection](unique_ptr<GeneCollection>& g) {
		return g->get_name() == gene_collection->get_name();
	});
	ensure(it != gene_collections.end(),
			"Gene collection with name '" + gene_collection->get_name() + "' already exists",
			ErrorType::GENERIC
	);
	gene_collections.emplace_back(std::move(gene_collection));
}

void Database::save() {
	Serialization::save_to_binary(get_main_file(), *this);
}

/*
 * Validate everything that constructors could not have checked (uniqueness, ...)
 * Validate:
 * - unique(GeneCollection.name)
 * Some of these can be ensured via invariants... But uniqueness checks are preferably only done after done loading all data (e.g. when adding in bulk to the database)
 */

} // end namespace
