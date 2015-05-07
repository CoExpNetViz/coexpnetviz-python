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

#include "Cluster.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

Cluster::Cluster()
{
}

Cluster::Cluster(const string& name)
:	name(name)
{
}

void Cluster::add(Gene& gene) {
	genes.emplace_back(&gene);
}

bool Cluster::empty() const {
	return genes.empty();
}

Cluster::Genes::const_iterator Cluster::begin() const {
	return genes.begin();
}

Cluster::Genes::const_iterator Cluster::end() const {
	return genes.end();
}

Cluster::Genes::iterator Cluster::begin() {
	return genes.begin();
}

Cluster::Genes::iterator Cluster::end() {
	return genes.end();
}

string Cluster::get_name() const {
	return name;
}

}
