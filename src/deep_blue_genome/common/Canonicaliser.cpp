// Author: Tim Diels <timdiels.m@gmail.com>

#include "Canonicaliser.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;

namespace DEEP_BLUE_GENOME {

Canonicaliser::Canonicaliser(Database& database, const std::string& species)
:	species(species)
{
	auto species_ = database.get_species(species);
	if (species_->has_canonical_mapping()) {
		mapping = species_->get_canonical_mapping();
	}
}

std::vector<std::string> Canonicaliser::get(std::string gene) const {
	to_lower(gene);
	if (mapping && mapping->has(gene)) {
		return mapping->get(gene);
	}

	// Hardcoded mappings
	if (species == "Tomato") {
		// solyc*.num.num -> Solyc*
		static boost::regex re("solyc([^.]*).*", boost::regex::perl);
		gene = regex_replace(gene, re, "Solyc$1");
	}

	return vector<string>(&gene, (&gene)+1);
}

}
