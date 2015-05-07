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

#include "GeneVariant.h"
#include <boost/algorithm/string.hpp>


using namespace std;

namespace DEEP_BLUE_GENOME {

GeneVariant::GeneVariant()
:	dna_sequence(*this)
{
}

GeneVariant::~GeneVariant()
{
}

DNASequence& GeneVariant::get_dna_sequence() {
	return dna_sequence;
}

void GeneVariant::set_functional_annotation(std::string annotation) {
	boost::algorithm::trim(annotation);
	assert(!annotation.empty());
	functional_annotation = annotation;
}

std::ostream& operator<<(std::ostream& str, const GeneVariant& variant) {
	variant.print(str);
	return str;
}

} // end namespace
