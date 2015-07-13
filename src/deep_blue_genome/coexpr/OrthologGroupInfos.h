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

#include <unordered_set>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/range/adaptors.hpp>
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfo.h>
#include <deep_blue_genome/coexpr/OrthologGroupInfo.h>

namespace DEEP_BLUE_GENOME {

class Gene;
class GeneCollection;
class OrthologGroup;

namespace COEXPR {

class OrthologGroupInfos : public boost::noncopyable
{
public:
	typedef std::unordered_set<const Gene*> Genes;

	/**
	 * All genes present in a expression matrix
	 */
	OrthologGroupInfos(Genes&& gene_collections);

	OrthologGroupInfo& get(const OrthologGroup& group);

public:
	/**
	 * Get range of OrthologGroupInfo&, family infos of gene
	 */
	auto get(const Gene& gene) {
		auto get_info = make_function([this](const OrthologGroup* group) -> OrthologGroupInfo& {
			return get(*group);
		});
		return gene.get_ortholog_groups() | boost::adaptors::transformed(get_info);
	}

private:
	Genes genes;
	std::unordered_map<const OrthologGroup*, OrthologGroupInfo> groups;
};

}} // end namespace
