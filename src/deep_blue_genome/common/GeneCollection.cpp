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

#include <deep_blue_genome/common/stdafx.h>
#include "GeneCollection.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/GeneExpressionMatrix.h>
#include <deep_blue_genome/common/Clustering.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/common/GeneFamilyId.h>
#include <deep_blue_genome/common/GeneVariantsUnsupportedException.h>

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
		gene_parser_rules.emplace_back(node["match"].as<std::string>(), node["replace"].as<std::string>());
	}
	ensure(!gene_parser_rules.empty(),
			"Need to specify at least one gene parser for gene collection '" + name + "'",
			ErrorType::GENERIC
	);
}

GeneCollection::GeneCollection(Database& database)
:	database(&database), is_unknown(true), name("Unknown"), species("Unknown")
{
	gene_parser_rules.emplace_back("(.+?)", "$1");
}

Gene& GeneCollection::get_gene(const std::string& name) {
	assert(!name.empty());
	auto result = try_get_gene(name);
	if (result) {
		return *result;
	}
	else {
		assert(!is_unknown);
		throw NotFoundException("Gene not part of gene collection: " + name);
	}
}

Gene* GeneCollection::try_get_gene(const std::string& name_) {
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
		// Add gene since it does not exist yet
		gene_it = name_to_gene.emplace(
				name,
				make_unique<Gene>(name, *this)
		).first;

		// Warn if we don't truly know its gene collection
		if (is_unknown) {
			cout << "Warning: Couldn't match gene '" << name << "' to a gene collection, adding to unknown gene collection\n";
		}
	}

	if (splice_variant_id && *splice_variant_id != 1) {
		throw GeneVariantsUnsupportedException("Encountered gene variant id, gene variants are unsupported: " + name_);
	}

	auto& gene = gene_it->second;
	return gene.get();

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

std::string GeneCollection::get_species() const {
	return species;
}


} // end namespace
