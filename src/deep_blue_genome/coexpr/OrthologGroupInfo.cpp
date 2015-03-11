// Author: Tim Diels <timdiels.m@gmail.com>

#include "OrthologGroupInfo.h"
#include <deep_blue_genome/common/OrthologGroup.h>
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/coexpr/BaitGroups.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

OrthologGroupInfo::OrthologGroupInfo(OrthologGroup& group, const vector<GeneCollection*>& gene_collections)
:	group(group)
{
	for (auto& gene : group) {
		name += gene->get_name() + ";";
	}
}

vector<Gene*>::const_iterator OrthologGroupInfo::begin() const {
	return group.begin();
}

vector<Gene*>::const_iterator OrthologGroupInfo::end() const {
	return group.end();
}

string OrthologGroupInfo::get_name() const {
	return name;
}

void OrthologGroupInfo::add_bait_correlation(const Gene& bait, double correlation) {
	auto match_bait = [&bait](const BaitCorrelation& bait_correlation) {
		return &bait_correlation.get_bait() == &bait;
	};

	assert(find_if(bait_correlations.begin(), bait_correlations.end(), match_bait) == bait_correlations.end());

	bait_correlations.emplace_back(bait, correlation);
}

const vector<BaitCorrelation>& OrthologGroupInfo::get_bait_correlations() const {
	return bait_correlations;
}

void OrthologGroupInfo::init_bait_group(BaitGroups& groups) {
	string bait_group_name;
	for (auto& bait_correlation : get_bait_correlations()) {
		bait_group_name += bait_correlation.get_bait().get_name() + ";"; // TODO cut off the last ;
	}

	bait_group = &groups.get(bait_group_name);
}

BaitGroup& OrthologGroupInfo::get_bait_group() {
	return *bait_group;
}


}} // end namespace
