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

#pragma once

#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <iostream>
#include <deep_blue_genome/common/DNASequence.h>

namespace DEEP_BLUE_GENOME {

class GeneCollection;
class Gene;

/**
 * Something with a sequence, a gene collection, ...
 *
 * Abstracts away whether you're dealing with a Gene or a SpliceVariant.
 *
 * You may not have multiple instances of the same gene (=same name) in memory.
 */
class GeneVariant : public boost::noncopyable
{
public:
	virtual ~GeneVariant();

	DNASequence& get_dna_sequence();
	void set_functional_annotation(std::string);
	boost::optional<std::string> get_functional_annotation();

	virtual GeneCollection& get_gene_collection() const = 0;

	/**
	 * Get gene if gene, gene of variant otherwise
	 */
	virtual Gene& get_gene() = 0;

	/**
	 * Get gene if gene, or gene of first splice variant, throws otherwise
	 */
	virtual Gene& as_gene() = 0;

protected:
	GeneVariant();

	/**
	 * Print short human readable description of self to stream
	 */
	virtual void print(std::ostream&) const = 0;

private:
	friend std::ostream& operator<<(std::ostream&, const DEEP_BLUE_GENOME::GeneVariant&);

private:
	DNASequence dna_sequence;
	std::string functional_annotation;
};

std::ostream& operator<<(std::ostream&, const GeneVariant&);

} // end namespace

