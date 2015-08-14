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

#include <deep_blue_genome/coexpr/BaitGroup.h>

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

// TODO this kind of map with some default construction is a frequent pattern. Should apply some DRY to it
class BaitGroups : public boost::noncopyable
{
public:
	typedef std::unordered_map<std::string, BaitGroup> Groups;

	/**
	 * Get Group of gene
	 */
	BaitGroup& get(std::string name);

	Groups::iterator begin();
	Groups::iterator end();

private:
	Groups groups;
};

}} // end namespace
