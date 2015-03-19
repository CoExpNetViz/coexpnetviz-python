// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <deep_blue_genome/coexpr/BaitCorrelation.h>

namespace DEEP_BLUE_GENOME {

class OrthologGroup;
class GeneCollection;
class Gene;

namespace COEXPR {

class BaitGroup;
class BaitGroups;

/**
 * A group of orthologs
 */
class OrthologGroupInfo : public boost::noncopyable
{
public:
	/**
	 * @param genes Iterable of distinct genes
	 */
	OrthologGroupInfo(OrthologGroup& group, const std::vector<GeneCollection*>& gene_collections);

	bool operator==(const OrthologGroup& other) const = delete;

	std::string get_name() const;

	void add_bait_correlation(const Gene& target, const Gene& bait, double correlation);

	const std::vector<BaitCorrelation>& get_bait_correlations() const;

	/**
	 * Figures out which bait group it's part of
	 */
	void init_bait_group(BaitGroups& groups);

	BaitGroup& get_bait_group();

	std::vector<Gene*>::const_iterator begin() const;
	std::vector<Gene*>::const_iterator end() const;

private:
	OrthologGroup& group;
	std::vector<BaitCorrelation> bait_correlations;
	BaitGroup* bait_group;
	std::vector<const Gene*> correlating_genes; // genes in ortholog group which actually correlate with a bait
};

}} // end namespace
