// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneCollection.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneCollection::GeneCollection(GeneCollectionId id, Database& database)
:	id(id), database(database)
{
	// Load general
	{
		auto query = database.prepare("SELECT name, species, gene_web_page FROM gene_collection WHERE id = %0q");
		query.parse();
		auto result = query.store(id);

		if (result.num_rows() == 0) {
			throw NotFoundException((make_string() << "Gene collection with id " << id << " not found").str());
		}

		assert(result.num_rows() == 1);
		auto row = *result.begin();
		name = row[0].conv<std::string>("");
		species = row[1].conv<std::string>("");
		gene_web_page = row[2];
	}

	// Load parser rules
	{
		auto query = database.prepare("SELECT id FROM gene_parser_rule WHERE gene_collection_id = %0q");
		query.parse();
		auto result = query.store(id);
		assert(result.num_rows() > 0);
		for (auto row : result) {
			gene_parser_rules.emplace_back(row[0], database);
		}
	}
}

GeneCollection::GeneCollection(const std::string& name, const std::string& species, YAML::Node parser_rules,
		const NullableGeneWebPage& gene_web_page, Database& database)
:	id(0),
	name(name),
 	species(species),
 	gene_web_page(gene_web_page),
 	database(database)
{
	for (auto node : parser_rules) {
		NullableRegexGroup splice_variant_group;
		if (node["splice_variant_group"]) {
			splice_variant_group = node["splice_variant_group"].as<RegexGroup>();
		}
		else {
			splice_variant_group = mysqlpp::null;
		}

		gene_parser_rules.emplace_back(node["match"].as<std::string>(), node["replace"].as<std::string>(), splice_variant_group, database);
	}
}

GeneVariant GeneCollection::get_gene_variant(const std::string& name) {
	GeneVariant out;
	if (try_get_gene_variant(name, out)) {
		return out;
	}
	else {
		throw NotFoundException("Gene not part of a known gene collection: " + name);
	}
}

bool GeneCollection::try_get_gene_variant(const std::string& name_, GeneVariant& out) {
	// Parse name
	bool parsed = false;
	NullableSpliceVariantId splice_variant_id;
	std::string name = name_;
	for (auto& rule : gene_parser_rules) {
		parsed = rule.try_parse(name, splice_variant_id);
		if (parsed) {
			break;
		}
	}

	if (!parsed) {
		return false;
	}

	Gene gene;
	{
		// Select existing gene
		auto query = database.prepare("SELECT id, ortholog_group_id FROM gene WHERE LOWER(name) = LOWER(%0q)");
		query.parse();
		auto result = query.store(name);
		if (result.num_rows() > 0) {
			assert(result.num_rows() == 1);
			auto row = *result.begin();
			gene = Gene(row[0], id, name, row[1]);
		}
		else { // Else insert gene as it doesn't exist yet
			auto query = database.prepare("INSERT INTO gene (gene_collection_id, name) VALUES (%0q, %1q)");
			query.parse();
			auto result = query.execute(id, name);
			gene = Gene(result.insert_id(), id, name, mysqlpp::null);
		}
	}

	// Select existing gene variant
	{
		auto query = database.prepare("SELECT id FROM gene_variant WHERE gene_id = %0q AND splice_variant_id = %1q");
		query.parse();
		auto result = query.store(gene.get_id(), splice_variant_id);
		if (result.num_rows() > 0) {
			assert(result.num_rows() == 1);
			auto row = *result.begin();
			out = GeneVariant(row[0], gene, splice_variant_id);
			return true;
		}
	}

	// Else insert gene variant as it doesn't exist yet
	{
		auto query = database.prepare("INSERT INTO gene_variant (gene_id, splice_variant_id) VALUES (%0q, %1q)");
		query.parse();
		auto result = query.execute(gene.get_id(), splice_variant_id);
		out = GeneVariant(result.insert_id(), gene, splice_variant_id);
		return true;
	}

	assert(false);
}

std::string GeneCollection::get_name() const {
	return name;
}

bool GeneCollection::has_gene_web_page() const {
	return !gene_web_page.is_null;
}

std::string GeneCollection::get_gene_web_page() const {
	assert(has_gene_web_page());
	return gene_web_page.data;
}

void GeneCollection::database_insert() {
	assert(id == 0);
	auto query = database.prepare("INSERT INTO gene_collection(name, species, gene_web_page) VALUES (%0q, %1q, %2q)");
	query.parse();
	auto result = query.execute(name, species, gene_web_page);
	id = result.insert_id();

	for (auto& rule : gene_parser_rules) {
		rule.database_insert();
	}
}

GeneCollectionId GeneCollection::get_id() const {
	return id;
}


} // end namespace
