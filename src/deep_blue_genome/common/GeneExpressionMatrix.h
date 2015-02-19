// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <memory>
#include <odb/lazy-ptr.hxx>
#include <deep_blue_genome/common/ublas.h>
#include <deep_blue_genome/common/types.h>
#include <deep_blue_genome/common/GeneCollection.h>

// TODO size_type is unsigned long long or such, we don't need thaaat much. Should swap it for a typedef of our own and then set that to uint32_t. You probably won't need more than uint, but there's not much extra effort in using a typedef
namespace DEEP_BLUE_GENOME {

class GeneExpressionMatrixClustering;
class Gene;
class Database;

typedef uint32_t GeneExpressionMatrixRow;

#pragma db object
// TODO each row label was originally a probe id, does that map to a gene in general, or rather a specific gene variant? What does it measure specifically? Currently we assume each row is a .1 gene
/**
 * Gene expression matrix
 *
 * matrix(i,j) = expression of gene i under condition j
 *
 * Note: these row indices are often used as gene ids in deep blue genome apps
 */
class GeneExpressionMatrix : public boost::noncopyable, public std::enable_shared_from_this<GeneExpressionMatrix>
{
public:
	/**
	 * Load from database
	 */
	GeneExpressionMatrix(GeneExpressionMatrixId matrix_id, Database&);

	/**
	 * Construct an expression matrix from a TODO particular plain text format
	 *
	 * @param species_name Name of species to which the gene expressions belong
	 */
	GeneExpressionMatrix(const std::string& name, const std::string& path, Database&);

	/**
	 * Get index of row corresponding to given gene
	 */
	GeneExpressionMatrixRow get_gene_row(GeneId) const;
	GeneId get_gene_id(GeneExpressionMatrixRow) const;
	bool has_gene(GeneId) const;

	std::string get_name() const;
	std::string get_species_name() const;

	void dispose_expression_data();

	/**
	 * Get inner matrix representation
	 */
	const matrix& get() const;

	/**
	 * Insert into database
	 */
	void database_insert();

private:
	friend class odb::access;

	GeneExpressionMatrix() {};  // for ODB

	#pragma db id auto
	GeneExpressionMatrixId id;

	#pragma db not_null
	std::shared_ptr<GeneCollection> gene_collection;

	std::string name; // name of dataset

	#pragma db transient
	matrix expression_matrix; // row_major

	// TODO db map... uh... eh?
	// TODO pragma unordered or stuff maaaybe needed?
	std::unordered_map<GeneExpressionMatrixRow, odb::lazy_shared_ptr<Gene>> gene_row_to_id;

	#pragma db transient
	std::unordered_map<GeneId, GeneExpressionMatrixRow> gene_id_to_row;
};

}
