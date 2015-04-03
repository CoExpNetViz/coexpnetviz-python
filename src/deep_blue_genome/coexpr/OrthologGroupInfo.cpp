// Author: Tim Diels <timdiels.m@gmail.com>

#include "OrthologGroupInfo.h"
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/util/template_magic.h>
#include <deep_blue_genome/coexpr/BaitGroups.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using namespace boost::adaptors;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

OrthologGroupInfo::OrthologGroupInfo(OrthologGroup& group, const vector<GeneCollection*>& gene_collections)
:	group(group)
{
	auto is_allowed_species = [&gene_collections](const Gene* g) {
		return contains(gene_collections, &g->get_gene_collection());
	};
	boost::insert(genes, group.get_genes() | filtered(make_function(is_allowed_species)));
}

const OrthologGroupInfo::Genes& OrthologGroupInfo::get_genes() const {
	return genes;
}

string OrthologGroupInfo::get_name() const {
	vector<string> names;
	for (auto& gene : correlating_genes) {
		names.emplace_back(gene->get_name());
	}
	sort(names.begin(), names.end());
	names.erase(unique(names.begin(), names.end()), names.end());

	string name;
	bool first=true;
	for (auto& name_ : names) {
		if (!first) {
			name += ";";
		}
		name += name_;
		first = false;
	}
	return name;
}

void OrthologGroupInfo::add_bait_correlation(const Gene& target, const Gene& bait, double correlation) {
	auto match_bait = [&bait](const BaitCorrelations& bait_correlation) {
		return &bait_correlation.get_bait() == &bait;
	};

	auto it = find_if(bait_correlations.begin(), bait_correlations.end(), match_bait);
	if (it == bait_correlations.end()) {
		bait_correlations.emplace_back(bait);
		it = bait_correlations.end()-1;
	}

	it->add_correlation(target, correlation);
	correlating_genes.emplace(&target);
}

const vector<BaitCorrelations>& OrthologGroupInfo::get_bait_correlations() const {
	return bait_correlations;
}

void OrthologGroupInfo::init_bait_group(BaitGroups& groups) {
	string bait_group_name;
	for (auto& bait_correlation : get_bait_correlations()) {
		bait_group_name += bait_correlation.get_bait().get_name() + ";"; // TODO cut off the last ;
	}

	bait_group = &groups.get(bait_group_name);
}

BaitGroup& OrthologGroupInfo::get_bait_group() const {
	return *bait_group;
}

const OrthologGroupInfo::Genes& OrthologGroupInfo::get_correlating_genes() const {
	return correlating_genes;
}

OrthologGroup::ExternalIds OrthologGroupInfo::get_external_ids() const {
	return group.get_external_ids();
}

const OrthologGroup::ExternalIdsGrouped& OrthologGroupInfo::get_external_ids_grouped() const {
	return group.get_external_ids_grouped();
}


}} // end namespace
