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

#include <deep_blue_genome/common/Serialization.h>

namespace DEEP_BLUE_GENOME {

class GeneFamilyId
{
public:
	GeneFamilyId(std::string source, std::string id);

	/**
	 * Get name of data set from which this family originates
	 */
	std::string get_source() const;

	/**
	 * Get external id
	 */
	std::string get_id() const;

	bool operator==(const GeneFamilyId&) const;
	bool operator<(const GeneFamilyId&) const;

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	GeneFamilyId();

private:
	std::string source;
	std::string id; // unique id within the scope of source (i.e. (source, id) is globally unique)
};

std::ostream& operator<<(std::ostream&, const GeneFamilyId&);

} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void GeneFamilyId::serialize(Archive& ar, const unsigned int version) {
	ar & source;
	ar & id;
}

}
