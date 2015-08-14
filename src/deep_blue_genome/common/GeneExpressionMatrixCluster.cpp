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

#include <deep_blue_genome/common/stdafx.h>
#include "GeneExpressionMatrixCluster.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

GeneExpressionMatrixCluster::GeneExpressionMatrixCluster(string name)
:	name(name)
{
}

void GeneExpressionMatrixCluster::add(GeneExpressionMatrixRow gene_index) {
	genes.emplace_back(gene_index);
}

bool GeneExpressionMatrixCluster::empty() const {
	return genes.empty();
}

std::vector<GeneExpressionMatrixRow>::const_iterator GeneExpressionMatrixCluster::begin() const {
	return genes.begin();
}

std::vector<GeneExpressionMatrixRow>::const_iterator GeneExpressionMatrixCluster::end() const {
	return genes.end();
}

std::vector<GeneExpressionMatrixRow>::iterator GeneExpressionMatrixCluster::begin() {
	return genes.begin();
}

std::vector<GeneExpressionMatrixRow>::iterator GeneExpressionMatrixCluster::end() {
	return genes.end();
}

string GeneExpressionMatrixCluster::get_name() const {
	return name;
}

}
