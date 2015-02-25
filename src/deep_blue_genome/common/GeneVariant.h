// Author: Tim Diels <timdiels.m@gmail.com>

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

