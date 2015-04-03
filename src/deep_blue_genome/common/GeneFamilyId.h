// Author: Tim Diels <timdiels.m@gmail.com>

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
