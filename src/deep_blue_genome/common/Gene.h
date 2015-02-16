// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

class Database;

class Gene
{
public:
	/**
	 * Construct invalid gene, a null value
	 */
	Gene();

	Gene(GeneId, GeneCollectionId, const std::string& name, NullableOrthologGroupId);
	Gene(GeneId, Database&);

	GeneId get_id() const;
	GeneCollectionId get_gene_collection_id() const;
	std::string get_name() const;

	bool has_orthologs() const;
	OrthologGroupId get_ortholog_group_id() const;

	bool operator<(const Gene&) const;

private:
	GeneId id;
	GeneCollectionId gene_collection_id; // genes collection which this gene is part of
	std::string name; // unique name of gene
	NullableOrthologGroupId ortholog_group_id;

	// TODO exons: with sequence_begin,end for each exon; referring to the sequence data in Genome
};


} // end namespace


namespace std {
template <> struct hash<DEEP_BLUE_GENOME::Gene>  // Template syntax, pretty as ever
{
	size_t operator()(const DEEP_BLUE_GENOME::Gene& x) const
	{
		size_t hash = 0;
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_id());
		return hash;
	}
};
} // end namespace
