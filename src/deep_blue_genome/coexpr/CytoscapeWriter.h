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

#include <boost/noncopyable.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <deep_blue_genome/coexpr/Node.h>

namespace DEEP_BLUE_GENOME {

class Gene;

namespace COEXPR {

class OrthologGroupInfo;
class OrthologGroupInfos;

typedef std::pair<const Gene*, const Gene*> BaitBaitOrthRelation;


/**
 * Writes out cytoscape files
 */
class CytoscapeWriter : public boost::noncopyable
{
public:
	CytoscapeWriter(std::string install_dir, const std::vector<Gene*>& baits, const std::vector<OrthologGroupInfo*>& neighbours, OrthologGroupInfos& groups);

	void write();

private:
	std::vector<BaitBaitOrthRelation>& get_bait_orthology_relations();

	// sif file related
	void write_sif();

	// edge attr file
	void write_edge_attr();

	// genes.yaml
	void write_genes();
	YAML::Node get_bait_node(const Gene& gene);
	YAML::Node get_family_node(const OrthologGroupInfo&);

	// node attr file
	void write_node_attr();
	void write_node_attr_baits(std::ostream& out);
	void write_node_attr_targets(std::ostream& out);

	template <class GeneRange, class FamiliesRange>
	void write_node_attr(std::ostream& out, const Node& node, bool bait, const GeneRange& gene_names, const FamiliesRange& families, const std::string& species, const std::string& colour);

private:
	std::string install_dir;
	const std::vector<Gene*>& baits;
	const std::vector<OrthologGroupInfo*>& neighbours;
	OrthologGroupInfos& groups;
	const std::string network_name;

	std::unordered_map<const Gene*, Node> bait_nodes;
	std::unordered_map<const OrthologGroupInfo*, Node> target_nodes;

	// get_bait_orthology_relations
	bool bait_orthologies_cached;
	std::vector<BaitBaitOrthRelation> bait_orthologies;
};


}} // end namespace
