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

#include <deep_blue_genome/morph/Ranking.h>

namespace DEEP_BLUE_GENOME {
namespace MORPH {

/**
 * Ranking for GOI
 */
class GOIResult : public boost::noncopyable
{
public:
	GOIResult()
	:	ausr_sum(0.0), ausr_count(0.0)
	{
	}

	void add_ausr(double ausr) {
		ausr_sum += ausr;
		ausr_count++;
	}

	double get_average_ausr() {
		return ausr_sum / ausr_count; // TODO numerically stable mean
	}

public:
	std::unique_ptr<Ranking> best_ranking;

private:
	double ausr_sum; // sum of all ausr
	double ausr_count; // number of AUSRs added
};

}}
