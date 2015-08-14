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

#include <deep_blue_genome/coexpr/stdafx.h>
#include "OrthologGroupInfo.h"
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/util/functional.h>
#include <deep_blue_genome/coexpr/BaitGroups.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using namespace boost::adaptors;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

OrthologGroupInfo::OrthologGroupInfo(const OrthologGroup& group)
:	group(group)
{
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

void OrthologGroupInfo::add_bait_correlation(Gene& target, Gene& bait, double correlation) {
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

OrthologGroup::ExternalIdsGrouped OrthologGroupInfo::get_external_ids_grouped() const {
	return group.get_external_ids_grouped();
}

const OrthologGroup::ExternalIds& OrthologGroupInfo::get_external_ids() const {
	return group.get_external_ids();
}

const OrthologGroup& OrthologGroupInfo::get() const {
	return group;
}


}} // end namespace
