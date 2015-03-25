// Author: Tim Diels <timdiels.m@gmail.com>

#include "CytoscapeWriter.h"
#include <boost/spirit/include/karma.hpp>
#include <iostream>
#include <fstream>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/coexpr/Baits.h>
#include <deep_blue_genome/coexpr/BaitCorrelations.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfo.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfos.h>
#include <deep_blue_genome/coexpr/BaitGroups.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
namespace karma = boost::spirit::karma;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

/*CytoscapeWriter::CytoscapeWriter(string install_dir, const vector<Gene*>& baits, const std::vector<OrthologGroupInfo*>& neighbours, unique_ptr<OrthologGroupInfos> groups)
{
}*/

/**
 * Write out a cytoscape network
 */
void CytoscapeWriter::write(string install_dir, const vector<Gene*>& baits, const std::vector<OrthologGroupInfo*>& neighbours, OrthologGroupInfos* groups) {
	using karma::format;
	auto sif_line = karma::string << "\t" << karma::string << "\t" << (karma::string % "\t") << "\n";
	auto edge_line = karma::string << " (" << karma::string << ") " << karma::string << "\t" << karma::double_ << "\n";

	// TODO big function, could use splitting into subfunctions
	std::string network_name = "network";

	ofstream out_sif(network_name + ".sif");
	out_sif.exceptions(ofstream::failbit | ofstream::badbit);

	ofstream out_edge_attr(network_name + ".edge.attr");
	out_edge_attr.exceptions(ofstream::failbit | ofstream::badbit);
	out_edge_attr << "link\tr_value\n";

	ofstream out_node_attr(network_name + ".node.attr");
	out_node_attr.exceptions(ofstream::failbit | ofstream::badbit);
	out_node_attr << "Gene\tCorrelation_to_baits\tType\tNode_Information\tColor\tSpecies\tHomologs\n";

	// TODO copy vizmap, without using boost copy_file (or try with a new header)

	// output bait orthology
	/*{
		// get a set of bait groups
		vector<OrthologGroupInfo*> bait_groups;
		for (auto bait : baits) {
			bait_groups.emplace_back(groups.get(*bait));
		}
		erase_duplicates(bait_groups);

		// output hom relation
		for (auto group : bait_groups) {
			// filter out genes not present in our set of baits
			vector<Gene*> genes;
			for (auto gene : group) {
				if (contains(baits, gene)) {
					genes.emplace_back(gene);
				}
			}

			// output all pairs
			for (auto it = genes.begin(); it != genes.end(); it++) {
				out_sif
				for (auto it2 = it+1; it != genes.end(); it2++) {
					out_sif
				}
			}
		}

	}*/

	// output bait node attributes
	for (auto bait : baits) {
		out_node_attr << bait->get_name() << "\t"; // TODO this should be 'Id', meaning node id
		out_node_attr << "\t"; // Skip: correlations to bait
		out_node_attr << "Bait\t";
		out_node_attr << "\t"; // Skip: function annotation (TODO this column will be removed later)
		out_node_attr << "#FFFFFF\t";
		out_node_attr << bait->get_gene_collection().get_species() << "\t";
		// Skip: homologs
		out_node_attr << "\n";
	}

	// bait homologs TODO this would only make sense if actually adding homologs of baits
	/*for (auto bait : baits) {
		OrthologGroup* group = bait.get_ortholog_group();
		if (group) {
			out_edge_attr << bait->get_name() << "\t";
			out_node_attr << "\t";
			out_node_attr << "Bait\t";
			out_node_attr << "\n";
		}
	}*/

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
		// cor relation between targets and baits
		if (!neigh->get_bait_correlations().empty()) {
			// out to sif
			{
				vector<std::string> bait_names;
				for (auto& bait_correlation : neigh->get_bait_correlations()) {
					auto bait_name = bait_correlation.get_bait().get_name();
					bait_names.emplace_back(bait_name);
				}
				out_sif << format(sif_line, neigh->get_name(), "cor", bait_names);
			}

			// out to edge attr
			for (auto& bait_correlation : neigh->get_bait_correlations()) {
				auto bait_name = bait_correlation.get_bait().get_name();
				out_edge_attr << format(edge_line,
						neigh->get_name(), "cor", bait_name, bait_correlation.get_max_correlation());
			}
		}

		// node attr
		out_node_attr << neigh->get_name() << "\t";

		{
			bool first = true;
			for (auto& bait_correlations : neigh->get_bait_correlations()) {
				if (!first) {
					out_node_attr << " ";
				}
				out_node_attr << bait_correlations.get_bait().get_name();
				first = false;
			}
			out_node_attr << "\t";
		}

		auto node_line = karma::string << " (" << karma::string << ") " << karma::string << "\t" << "\n";

		out_node_attr << "Target\t";
		out_node_attr << "\t"; // Skipping func annotation for now (col will be removed later)
		out_node_attr << neigh->get_bait_group().get_colour() << "\t";
		out_node_attr << "\t"; // Skip species, these are of multiple
		out_node_attr << "\n";
	}
}

}} // end namespace
