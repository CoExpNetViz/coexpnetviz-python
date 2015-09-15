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
#include "Gene.h"
#include <deep_blue_genome/common/OrthologGroup.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

Gene::Gene()
:	gene_collection(nullptr)
{
}

Gene::Gene(const std::string& name, GeneCollection& gene_collection)
:	name(name), gene_collection(&gene_collection)
{
}

GeneCollection& Gene::get_gene_collection() const {
	return *gene_collection;
}

std::string Gene::get_name() const {
	return name;
}

void Gene::print(std::ostream& str) const {
	str << "gene '" << name << "'";
}

void Gene::set_functional_annotation(std::string annotation) {
	boost::algorithm::trim(annotation);
	assert(!annotation.empty());
	functional_annotation = annotation;
}

std::ostream& operator<<(std::ostream& str, const Gene& gene) {
	gene.print(str);
	return str;
}

void Gene::add_highly_similar(Gene& gene) {
	if (contains(highly_similar_genes, &gene))
		return;

	ensure(&get_gene_collection() != &gene.get_gene_collection(),
			(make_string() << "Encountered mapping between 2 genes of the same gene collection: " << *this << ", " << gene).str(), // TODO is that a problem?
			ErrorType::GENERIC
	);

	highly_similar_genes.emplace_back(&gene);
}

boost::optional<std::string> Gene::get_functional_annotation() {
	return functional_annotation;
}

} // end namespace
