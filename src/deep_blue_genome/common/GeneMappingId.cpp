// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneMappingId.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/TabGrammarRules.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

GeneMappingId::GeneMappingId(const std::string& source_species, const std::string& target_species)
:	source_species(source_species), target_species(target_species)
{
}

const std::string& GeneMappingId::get_source_species() const {
	return source_species;
}

const std::string& GeneMappingId::get_target_species() const {
	return target_species;
}

bool GeneMappingId::operator<(const GeneMappingId& other) const {
	if (source_species < other.source_species)
		return true;
	else
		return target_species < other.target_species;
}

bool GeneMappingId::operator==(const GeneMappingId& other) const {
	return source_species == other.source_species && target_species == other.target_species;
}

}
