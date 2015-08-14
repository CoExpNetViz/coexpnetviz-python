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

#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/ublas.h>

namespace DEEP_BLUE_GENOME {

class GeneCollection;
class Gene;
class DataFileImport;

// TODO each row label was originally a probe id, does that map to a gene in general, or rather a specific gene variant? What does it measure specifically? Currently we assume each row is a .1 gene
/**
 * Gene expression matrix
 *
 * A matrix can contain genes of multiple gene collections
 *
 * matrix(i,j) = expression of gene i under condition j
 *
 * Note: these row indices are often used as gene ids in deep blue genome apps
 */
class GeneExpressionMatrix : public boost::noncopyable
{
	friend class DataFileImport;

public:
	/**
	 * Construct invalid instance
	 */
	GeneExpressionMatrix();

	/**
	 * Get index of row corresponding to given gene
	 */
	GeneExpressionMatrixRow get_gene_row(Gene&) const;
	Gene& get_gene(GeneExpressionMatrixRow) const;
	bool has_gene(const Gene&) const;

	/**
	 * Get range of genes in matrix
	 *
	 * @return concept Range<Gene*>
	 */
	auto get_genes() const {
		return gene_to_row | boost::adaptors::map_keys;
	}

	std::string get_name() const;

	/**
	 * Get inner matrix representation
	 */
	const matrix& get() const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::string name; // name of dataset
	matrix expression_matrix; // row_major
	std::vector<Gene*> row_to_gene; // row_to_gene[row] = gene
	std::unordered_map<Gene*, GeneExpressionMatrixRow> gene_to_row; // inverse of gene_row_to_id
};

}

std::ostream& operator<<(std::ostream&, const DEEP_BLUE_GENOME::GeneExpressionMatrix&);


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void GeneExpressionMatrix::serialize(Archive& ar, const unsigned int version) {
	ar & name;
	ar & expression_matrix; // TODO might want to lazy load this, or the class instance itself
	ar & row_to_gene;

	if (Archive::is_loading::value) {
		for (auto p : row_to_gene | boost::adaptors::indexed(0)) {
			gene_to_row.emplace(p.value(), p.index());
		}
	}
}

}
