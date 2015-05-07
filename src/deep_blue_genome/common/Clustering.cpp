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

#include "Clustering.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

Clustering::Clustering()
:	gene_collection(nullptr), expression_matrix(nullptr)
{
}

GeneCollection& Clustering::get_gene_collection() const {
	return *gene_collection;
}

Clustering::const_iterator Clustering::begin() const {
	return clusters.begin();
}

Clustering::const_iterator Clustering::end() const {
	return clusters.end();
}

std::string Clustering::get_name() const {
	return name;
}

}
