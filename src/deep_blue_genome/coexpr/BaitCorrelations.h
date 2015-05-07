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

#include <vector>
#include <utility>

namespace DEEP_BLUE_GENOME {

class Gene;

namespace COEXPR {

/**
 * Correlations to a bait gene
 *
 * Data class of Group.
 */
class BaitCorrelations {
public:
	BaitCorrelations(const Gene& bait);

	const Gene& get_bait() const;
	double get_max_correlation() const;
	void add_correlation(const Gene& target, double correlation);

private:
	const Gene& bait;
	std::vector<std::pair<const Gene*, double>> correlations; // correlation between target and bait
};

}} // end namespace
