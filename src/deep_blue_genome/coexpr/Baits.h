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

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

/**
 * A collection of genes, possibly of various species
 */
class Baits // TODO quite similar to GOI in the sense that applying a gene mapping to it is interesting
{
public:
	/**
	 * @param path Path to plain text TODO describe format (same as GOI)
	 */
	Baits(std::string path);
	const std::vector<std::string>& get_genes() const;

private:
	std::vector<std::string> genes;
};

}}
