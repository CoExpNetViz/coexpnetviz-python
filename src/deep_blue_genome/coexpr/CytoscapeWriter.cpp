// Author: Tim Diels <timdiels.m@gmail.com>

#include "CytoscapeWriter.h"
#include <iostream>
#include <fstream>
#include <functional>
#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/join.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/container/set.hpp>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/writer/OrthologGroup.h>
#include <deep_blue_genome/coexpr/Baits.h>
#include <deep_blue_genome/coexpr/BaitCorrelations.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfo.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfos.h>
#include <deep_blue_genome/coexpr/BaitGroups.h>
#include <deep_blue_genome/util/printer.h>
#include <deep_blue_genome/util/functional.h>

using namespace DEEP_BLUE_GENOME;
using namespace DEEP_BLUE_GENOME::COMMON::WRITER;
using namespace boost::adaptors;
using namespace std;
using boost::container::flat_set;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

uint64_t Node::next_id = 1;

CytoscapeWriter::CytoscapeWriter(std::string install_dir, const std::vector<Gene*>& baits, const std::vector<OrthologGroupInfo*>& neighbours, OrthologGroupInfos& groups)
:	install_dir(install_dir), baits(baits), neighbours(neighbours), groups(groups), network_name("network"), bait_orthologies_cached(false)
{
}

/**
 * Get orthology relations between baits
 */
std::vector<BaitBaitOrthRelation>& CytoscapeWriter::get_bait_orthology_relations() {
	if (!bait_orthologies_cached) {
		// get a set of ortholog groups of the baits
		vector<OrthologGroupInfo*> bait_groups;
		for (auto bait : baits) {
			bait_groups.emplace_back(&groups.get(*bait));
		}
		erase_duplicates(bait_groups);

		// enumerate all possible pairs of baits of the same group
		for (auto group : bait_groups) {
			// filter out genes not present in our set of baits
			vector<const Gene*> genes;
			assert(group);
			for (auto gene : group->get_genes()) {
				if (contains(baits, gene)) {
					genes.emplace_back(gene);
				}
			}

			// output all pairs
			for (auto it = genes.begin(); it != genes.end(); it++) {
				for (auto it2 = it+1; it2 != genes.end(); it2++) {
					bait_orthologies.emplace_back(make_pair(*it, *it2));
					bait_orthologies.emplace_back(make_pair(*it2, *it));
				}
			}
		}

		bait_orthologies_cached = true;
	}

	return bait_orthologies;
}

/**
 * Write out a cytoscape network
 */
void CytoscapeWriter::write() {
	// write data to each file
	write_sif();
	write_node_attr();
	write_edge_attr();
	write_genes();

	// copy additional assets into the network's directory
	// copy_vizmap() TODO, without using boost copy_file (or try with a new header)
}

/**
 * Write node attributes
 *
 * Particularly, attributes of:
 * - bait nodes
 * - target nodes
 */
void CytoscapeWriter::write_node_attr() {
	ofstream out(network_name + ".node.attr");
	out.exceptions(ofstream::failbit | ofstream::badbit);
	auto fields = {"node_id", "families", "genes", "species", "color"};
	out << intercalate("\t", fields) << "\n";

	write_node_attr_baits(out);
	write_node_attr_targets(out);
}

void CytoscapeWriter::write_node_attr_baits(ostream& out) {
	std::string colour = "#FFFFFF";
	for (auto bait : baits) {
		// fetch data
		std::string gene_names[] = {bait->get_name()};

		// write
		write_node_attr(out, bait_nodes[bait], gene_names, groups.get(*bait).get_external_ids_grouped(), bait->get_gene_collection().get_species(), colour);
	}
}

void CytoscapeWriter::write_node_attr_targets(ostream& out) {
	// have each neigh figure out what its bait group is
	BaitGroups bait_groups;
	for (auto neigh : neighbours) {
		neigh->init_bait_group(bait_groups);
	}

	// assign colours to bait groups
	std::default_random_engine generator;
	std::uniform_int_distribution<int> distribution(0, 0x00FFFFFF);
	for (auto& p : bait_groups) {
		auto& group = p.second;

		ostringstream str;
		int colour = distribution(generator);
		str << "#";
		str.width(6);
		str.fill('0');
		str << std::hex << colour;

		group.set_colour(str.str());
	}

	// output targets (= family nodes)
	for (auto neigh : neighbours) {
		// fetch data
		auto get_name = make_function([](const Gene* g) { // TODO could use std bind or something?
			return g->get_name();
		});
		auto gene_names = neigh->get_correlating_genes() | transformed(get_name);

		// write
		write_node_attr(out, target_nodes[neigh], gene_names, neigh->get_external_ids_grouped(), "", neigh->get_bait_group().get_colour());
	}
}

/**
 * Write out attributes of single node
 *
 * @param gene_names Names of genes in node
 * @param family_names_by_source All external ids by source of associated ortholog group
 * @param species Species name of bait if a bait node, empty string otherwise
 * @param colour Colour of node
 */
template <class GeneRange, class IdsRange>
void CytoscapeWriter::write_node_attr(ostream& out, const Node& node, GeneRange&& gene_names, IdsRange&& external_ids_grouped, const std::string& species, const std::string& colour) {
	// gene_names
	auto genes = intercalate(" ", std::forward<GeneRange>(gene_names));

	// family_names_by_source -> families
	auto get_id = make_function([](const GeneFamilyId& id) {
		return id.get_id();
	});
	typedef std::pair<std::string, boost::container::flat_set<GeneFamilyId>> IdSubset;
	auto get_family_string = make_function([&get_id](const IdSubset& p) {
		return make_printer([p, &get_id](ostream& out) {
			out << "From " << p.first << ": " << intercalate(", ", p.second | transformed(get_id));
		});
	});
	auto families = intercalate(". ", std::forward<IdsRange>(external_ids_grouped) | transformed(get_family_string));

	// output attr line
	out << intercalate_("\t", node, families, genes, species, colour) << "\n";
}

/**
 * Write yaml file with info of each gene
 */
void CytoscapeWriter::write_genes() {
	YAML::Node root;

	// baits to yaml nodes
	for (auto gene : baits) {
		root["genes"].push_back(get_gene_node(*gene, true));
	}

	// collect targets
	flat_set<const Gene*> targets;
	for (auto neigh : neighbours) {
		boost::insert(targets, neigh->get_correlating_genes());
	}

	// targets to yaml nodes
	for (auto gene : targets) {
		root["genes"].push_back(get_gene_node(*gene, false));
	}

	// Write to file
	ofstream out(network_name + "_genes.yaml");
	out.exceptions(ofstream::failbit | ofstream::badbit);
	out << YAML::Dump(root);
}

YAML::Node CytoscapeWriter::get_gene_node(const Gene& gene, bool is_bait) {
	auto&& ortho_group = groups.get(gene);

	YAML::Node gene_;
	gene_["id"] = gene.get_name(); // matches gene ids used in the node attr file (genes column)
	// gene_["go_terms"] = TODO;
	gene_["is_bait"] = is_bait;
	if (is_bait) {
		// families
		gene_["families"] = write_yaml(ortho_group.get());

		// orthologs
		auto is_not_gene = make_function([&gene](const Gene* g) {
			return g != &gene;
		});
		auto get_name = make_function([](const Gene* g) {
					return g->get_name();
		});
		vector<std::string> orthologs;
		boost::push_back(orthologs, ortho_group.get_correlating_genes() | filtered(is_not_gene) | transformed(get_name));
		gene_["orthologs"] = orthologs;
	}
	else {
		// baits
		for (auto bait_correlation : ortho_group.get_bait_correlations()) {
			YAML::Node bait;
			bait["node_id"] = (make_string() << bait_nodes[&bait_correlation.get_bait()]).str();
			bait["r_value"] = bait_correlation.get_max_correlation();
			gene_["baits"].push_back(bait);
		}
	}

	return gene_;
}


/**
 * Write node relations
 *
 * Particularly:
 * - target->bait correlation relations
 * - bait <-> bait orthology relations
 */
void CytoscapeWriter::write_sif() {
	ofstream out(network_name + ".sif");
	out.exceptions(ofstream::failbit | ofstream::badbit);

	// target -> bait correlation
	for (auto& neigh : neighbours) {
		auto&& delimiter = "\t";
		auto&& prefix = intercalate_(delimiter, target_nodes[neigh], "cor");
		if (!neigh->get_bait_correlations().empty()) {
			auto get_name = make_function([this](const BaitCorrelations& bait_correlation) {
				return (make_string() << bait_nodes[&bait_correlation.get_bait()]).str();
			});
			auto&& bait_nodes_ = intercalate(delimiter, neigh->get_bait_correlations() | transformed(get_name));
			out << intercalate_(delimiter, prefix, bait_nodes_) << "\n";
		}
	}

	// bait <-> bait orthology
	for (auto& p : get_bait_orthology_relations()) {
		out << bait_nodes[p.first] << "\thom\t" << bait_nodes[p.second] << "\n";
		// TODO we are printing pairs per line, not: src rel dst1 dst2 .. dstN. Does that read stuff fine?
	}
}

/**
 * Write same node relations as sif, but in more detail
 */
void CytoscapeWriter::write_edge_attr() {
	ofstream out(network_name + ".edge.attr");
	out.exceptions(ofstream::failbit | ofstream::badbit);
	out << "edge\tr_value\n";

	// target -> bait correlation
	for (auto&& neigh : neighbours) {
		if (!neigh->get_bait_correlations().empty()) {
			for (auto& bait_correlation : neigh->get_bait_correlations()) {
				out << target_nodes[neigh] << " (cor) " << bait_nodes[&bait_correlation.get_bait()] << "\t" << bait_correlation.get_max_correlation() << "\n";
			}
		}
	}

	// bait <-> bait orthology
	for (auto& p : get_bait_orthology_relations()) {
		out << bait_nodes[p.first] << " (hom) " << bait_nodes[p.second] << "\tNA\n";
	}
}

}} // end namespace
