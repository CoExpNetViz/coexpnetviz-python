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
#include <boost/regex.hpp>
#include <deep_blue_genome/common/Canonicaliser.h>

namespace DEEP_BLUE_GENOME {
namespace MORPH {

/**
 * Genes of interest (GOI)
 *
 * A collection of genes.
 */
class GenesOfInterest
{
public:
	/**
	 * @param name Name of goi
	 * @param path Path to plain text TODO describe format
	 */
	GenesOfInterest(std::string name, std::string path, const boost::regex& gene_pattern);
	const std::vector<std::string>& get_genes() const;
	std::string get_name() const;

	/**
	 * Apply mappings to genes
	 */
	void canonicalise(const DEEP_BLUE_GENOME::Canonicaliser&);

private:
	std::string name;
	std::vector<std::string> genes;
};

}}
