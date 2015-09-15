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

#include <deep_blue_genome/common/database_all.h>

namespace DEEP_BLUE_GENOME {

class GeneExpressionMatrixClustering;
class GeneExpressionMatrixCluster;

namespace MORPH {

class GenesOfInterest;

/**
 * Private class of Ranking
 */
class Ranking_ClusterInfo : public boost::noncopyable { // TODO this could be tidier
public:
	Ranking_ClusterInfo(const DEEP_BLUE_GENOME::GeneCorrelationMatrix&, const boost::container::flat_set<GeneExpressionMatrixRow>& genes_of_interest, const DEEP_BLUE_GENOME::GeneExpressionMatrixCluster& cluster);

	auto get_goi_count() {
		return goi.size();
	}

	DEEP_BLUE_GENOME::indirect_array goi; // row indices of genes of interest in cluster
	DEEP_BLUE_GENOME::indirect_array goi_columns; // column indices of genes of interest in cluster
	DEEP_BLUE_GENOME::indirect_array candidates; // candidates in cluster (i.e. not goi)
	DEEP_BLUE_GENOME::indirect_array genes; // all genes in cluster

private:
	DEEP_BLUE_GENOME::array goi_;
	DEEP_BLUE_GENOME::array goi_columns_;
};

// TODO refactor
/**
 * Note: A negative ranking value for a gene means it wasn't ranked
 */
class Ranking : public boost::noncopyable // TODO this is not a single ranking, it's a set of rankings (but we've already used the name Rankings/rankings internally)
{
public:

	Ranking(boost::container::flat_set<GeneExpressionMatrixRow> genes_of_interest, DEEP_BLUE_GENOME::GeneExpressionMatrixClustering&, const DEEP_BLUE_GENOME::GeneCorrelationMatrix&, const std::string& name);

	/**
	 * Save top k results in given directory
	 *
	 * Full goi: goi without genes missing in dataset removed
	 */
	void save(std::string directory, int top_k, const GenesOfInterest& full_goi, double average_ausr, bool output_yaml) const;

	double get_ausr() const;
	bool operator>(const Ranking&) const;

private:
	typedef boost::numeric::ublas::vector<double> Rankings;

	const matrix& get_gene_correlations() const;
	const DEEP_BLUE_GENOME::GeneExpressionMatrix& get_gene_expression() const;
	void rank_genes(Rankings& rankings);
	void rank_self(const Rankings& rankings);
	void finalise_ranking(const Rankings& rankings);

	/**
	 * Finalise part of ranking
	 *
	 * Said part is: project(final_rankings, sub_indices)
	 * cluster info must be of the cluster it belongs to
	 *
	 * Note: this func is highly specialised, not very reusable
	 */
	void finalise_sub_ranking(const Rankings& rankings, Rankings& final_rankings, const DEEP_BLUE_GENOME::indirect_array& sub_indices, Ranking_ClusterInfo&, long excluded_goi = -1);

private:
	boost::container::flat_set<GeneExpressionMatrixRow> genes_of_interest;
	GeneExpressionMatrixClustering* clustering;
	const GeneCorrelationMatrix& gene_correlations; // Note: only valid during construction
	Rankings final_rankings; // finalised rankings, after ctor has finished
	double ausr;
	std::string name;
	std::unordered_map<const DEEP_BLUE_GENOME::GeneExpressionMatrixCluster*, Ranking_ClusterInfo> cluster_info; // TODO could use vector instead, uses just iterate over all of it
};

}}
