// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <memory>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/ublas.h> // TODO would compile speed up much without this in many headers?

// TODO size_type is unsigned long long or such, we don't need thaaat much. Should swap it for a typedef of our own and then set that to uint32_t. You probably won't need more than uint, but there's not much extra effort in using a typedef
namespace DEEP_BLUE_GENOME {

class GeneCollection;
class Gene;
class DataFileImport;

// TODO each row label was originally a probe id, does that map to a gene in general, or rather a specific gene variant? What does it measure specifically? Currently we assume each row is a .1 gene
/**
 * Gene expression matrix
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

	std::string get_name() const;
	std::string get_species_name() const;

	void dispose_expression_data();

	/**
	 * Get inner matrix representation
	 */
	const matrix& get() const;

	GeneCollection& get_gene_collection() const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	GeneCollection* gene_collection; // not null
	std::string name; // name of dataset
	matrix expression_matrix; // row_major
	std::unordered_map<GeneExpressionMatrixRow, Gene*> row_to_gene;
	std::unordered_map<Gene*, GeneExpressionMatrixRow> gene_to_row; // inverse of gene_row_to_id
};

}


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void GeneExpressionMatrix::serialize(Archive& ar, const unsigned int version) {
	ar & gene_collection;
	ar & name;
	ar & expression_matrix; // TODO might want to lazy load this, or the class instance itself
	ar & row_to_gene;

	if (Archive::is_loading::value) {
		for (auto& p : row_to_gene) {
			gene_to_row.emplace(p.second, p.first);
		}
	}
}

}
