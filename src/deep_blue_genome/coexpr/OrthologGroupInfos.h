// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/coexpr/OrthologGroupInfo.h>

namespace DEEP_BLUE_GENOME {

class Gene;
class GeneCollection;
class OrthologGroup;

namespace COEXPR {

class OrthologGroupInfos : public boost::noncopyable
{
public:
	typedef std::vector<GeneCollection*> GeneCollections;

	OrthologGroupInfos(GeneCollections gene_collections);

	/**
	 * Get Group of gene
	 */
	OrthologGroupInfo* get(const Gene& gene);

private:
	GeneCollections gene_collections;
	std::unordered_map<OrthologGroup*, OrthologGroupInfo> groups;
};

}} // end namespace
