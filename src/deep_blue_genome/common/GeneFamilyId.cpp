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

#include "GeneFamilyId.h"

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneFamilyId::GeneFamilyId()
{
}

GeneFamilyId::GeneFamilyId(std::string source, std::string id)
:	source(source), id(id)
{
}

std::string GeneFamilyId::get_source() const {
	return source;
}

std::string GeneFamilyId::get_id() const {
	return id;
}

bool GeneFamilyId::operator==(const GeneFamilyId& o) const {
	return id == o.id && source == o.source;
}

bool GeneFamilyId::operator<(const GeneFamilyId& o) const {
	if (source < o.source)
		return true;
	return id < o.id;
}

std::ostream& operator<<(std::ostream& out, const GeneFamilyId& id) {
	out << "(" << id.get_source() << ", " << id.get_id() << ")";
	return out;
}

} // end namespace
