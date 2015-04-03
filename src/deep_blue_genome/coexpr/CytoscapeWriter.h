// Author: Tim Diels <timdiels.m@gmail.com>

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
	YAML::Node get_gene_node(const Gene& gene, bool is_bait);

	// node attr file
	void write_node_attr();
	void write_node_attr_baits(std::ostream& out);
	void write_node_attr_targets(std::ostream& out);

	template <class GeneRange, class IdsRange>
	void write_node_attr(std::ostream& out, const Node& node, GeneRange&& gene_names, IdsRange&& family_names_by_source, const std::string& species, const std::string& colour);

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
