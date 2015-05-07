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

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/container/set.hpp>
#include <deep_blue_genome/common/OrthologGroup.h>
#include <deep_blue_genome/coexpr/BaitCorrelations.h>

namespace DEEP_BLUE_GENOME {

class GeneCollection;
class Gene;
class GeneFamilyId;

namespace COEXPR {

class BaitGroup;
class BaitGroups;

/**
 * A view on an ortholog group, filtered by a list of gene collections.
 *
 * I.e. genes not part of one of the listed collections are filtered out by this view.
 */
class OrthologGroupInfo : public boost::noncopyable // TODO this class is a mix of ortho group filtered by list of species, and things specific to a target node
{
public:
	typedef boost::container::flat_set<const Gene*> Genes;

public:
	OrthologGroupInfo(OrthologGroup& group, const std::unordered_set<const Gene*>& all_genes);

	bool operator==(const OrthologGroup& other) const = delete;

	std::string get_name() const;

	void add_bait_correlation(const Gene& target, const Gene& bait, double correlation);

	const std::vector<BaitCorrelations>& get_bait_correlations() const;

	/**
	 * Figures out which bait group it's part of
	 */
	void init_bait_group(BaitGroups& groups);

	BaitGroup& get_bait_group() const;

	/**
	 * Get external ids assigned to this groups
	 *
	 * @returns Range of pairs of (source, ids with matching source)
	 */
	OrthologGroup::ExternalIdsGrouped get_external_ids_grouped() const;

	const OrthologGroup::ExternalIds& get_external_ids() const;

	/**
	 * Get range of containing genes
	 */
	const Genes& get_genes() const;

	/**
	 * Get range of containing genes, which correlate with a bait
	 */
	const Genes& get_correlating_genes() const;

	const OrthologGroup& get() const;

private:
	OrthologGroup& group;
	std::vector<BaitCorrelations> bait_correlations;
	BaitGroup* bait_group;
	Genes correlating_genes; // genes in this->genes which actually correlate with a bait
	Genes genes; // genes in group, filtered by particular species
};

}} // end namespace
