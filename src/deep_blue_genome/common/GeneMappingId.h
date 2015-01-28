// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <deep_blue_genome/common/util.h>

namespace DEEP_BLUE_GENOME {

class GeneMappingId
{
public:
	GeneMappingId(const std::string& source_species, const std::string& target_species);

	const std::string& get_source_species() const;
	const std::string& get_target_species() const;

	bool operator<(const GeneMappingId& other) const;
	bool operator==(const GeneMappingId& other) const;

private:
	std::string source_species;
	std::string target_species;
};

} // end namespace


namespace std {
template <> struct hash<DEEP_BLUE_GENOME::GeneMappingId>
{
	size_t operator()(const DEEP_BLUE_GENOME::GeneMappingId& x) const
	{
		size_t hash = 0;
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_source_species());
		DEEP_BLUE_GENOME::hash_combine(hash, x.get_target_species());
		return hash;
	}
};
}
