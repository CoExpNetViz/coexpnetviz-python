// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <unordered_set>
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
	typedef std::unordered_set<const Gene*> Genes;

	/**
	 * All genes present in a expression matrix
	 */
	OrthologGroupInfos(Genes&& gene_collections);

	/**
	 * Get Group of gene
	 */
	OrthologGroupInfo& get(const Gene& gene);

private:
	Genes genes;
	std::unordered_map<OrthologGroup*, OrthologGroupInfo> groups;
};

}} // end namespace
