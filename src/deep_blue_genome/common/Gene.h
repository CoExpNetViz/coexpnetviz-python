// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

namespace DEEP_BLUE_GENOME {

class Gene
{
public:
	Gene(); // Construct invalid gene. Used as null value, todo: refactor
	Gene(GeneId, GeneCollectionId, const std::string& name);

	GeneId get_id() const;
	GeneId get_gene_collection_id() const;
	std::string get_name() const;

private:
	GeneId id;
	GeneCollectionId gene_collection_id; // genes collection which this gene is part of
	std::string name; // unique name of gene

	// TODO exons: with sequence_begin,end for each exon; referring to the sequence data in Genome
};


} // end namespace
