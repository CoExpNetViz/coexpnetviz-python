// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneCollection.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/SpliceVariant.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/Clustering.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneFamilyId.h>

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneCollection::GeneCollection()
:	database(nullptr)
{
}

void GeneCollection::init_serialised(Database& database) {
	this->database = &database;
}

GeneCollection::GeneCollection(Database& database, const std::string& name, const std::string& species, YAML::Node parser_rules,
		const NullableGeneWebPage& gene_web_page)
:	database(&database), is_unknown(false), name(name), species(species), gene_web_page(gene_web_page)
{
	for (auto node : parser_rules) {
		NullableRegexGroup splice_variant_group;
		if (node["splice_variant_group"]) {
			splice_variant_group = node["splice_variant_group"].as<RegexGroup>();
		}

		gene_parser_rules.emplace_back(node["match"].as<std::string>(), node["replace"].as<std::string>(), splice_variant_group);
	}
	ensure(!gene_parser_rules.empty(),
			"Need to specify at least one gene parser for gene collection '" + name + "'",
			ErrorType::GENERIC
	);
}

GeneCollection::GeneCollection(Database& database)
:	database(&database), is_unknown(true), name("Unknown"), species("Unknown")
{
	gene_parser_rules.emplace_back("(.*)([.]([0-9]+))?", "$1", 3);
}

GeneVariant& GeneCollection::get_gene_variant(const std::string& name) {
	auto result = try_get_gene_variant(name);
	if (result) {
		return *result;
	}
	else {
		throw NotFoundException("Gene not part of a known gene collection: " + name);
	}
}

GeneVariant* GeneCollection::try_get_gene_variant(const std::string& name_) {
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
		return nullptr;
	}

	// Get gene
	auto gene_it = name_to_gene.find(name);
	if (gene_it == name_to_gene.end()) {
		// Add gene if does not exist yet

		// start with a dummy ortholog group
		auto& ortho_group = database->add_ortholog_group();

		// now add gene
		gene_it = name_to_gene.emplace(
				name,
				make_unique<Gene>(name, *this, ortho_group)
		).first;

		// Warn if we don't truly know its gene collection
		if (is_unknown) {
			cerr << "Warning: Couldn't match gene '" << name << "' to a gene collection, adding to unknown gene collection\n";
		}
	}

	auto& gene = gene_it->second;

	if (!splice_variant_id) {
		return gene.get(); // gene was all that was requested
	}

	// Get splice variant of gene
	return &gene->get_splice_variant(*splice_variant_id);
}

std::string GeneCollection::get_name() const {
	return name;
}

NullableGeneWebPage GeneCollection::get_gene_web_page() const {
	return gene_web_page;
}

bool GeneCollection::operator==(const GeneCollection& other) const {
	return this == &other;
}

void GeneCollection::add_clustering(unique_ptr<Clustering>&& clustering) {
	ensure(clusterings.find(clustering->get_name()) == clusterings.end(),
			(make_string() << "Cannot add 2 clusterings with the same name '" << clustering->get_name() << "' to gene collection '" << name << "'").str(),
			ErrorType::GENERIC
	);

	std::string name_lower = clustering->get_name();
	to_lower(name_lower);
	clusterings[name_lower] = std::move(clustering);
}

std::string GeneCollection::get_species() const {
	return species;
}


} // end namespace
