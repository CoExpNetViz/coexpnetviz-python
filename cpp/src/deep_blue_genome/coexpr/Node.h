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

namespace DEEP_BLUE_GENOME {

namespace COEXPR {

class Node
{
public:
	Node();

	uint64_t get_id() const;

	/**
	 * Get node id for use in cytoscape
	 *
	 * Can't use numbers as node ids in cytoscape, see bug https://groups.google.com/d/msg/qiime-forum/_s83iQKnoAo/5_GgRhItbKgJ
	 */
	std::string get_cytoscape_id() const;

private:
	uint64_t id;
	static uint64_t next_id;
};


}} // end namespace
