// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneParserRule.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneParserRule::GeneParserRule()
{
}

GeneParserRule::GeneParserRule(const std::string& match, const std::string& replace, NullableRegexGroup group)
:	replace_format(replace), splice_variant_group(group)
{
	set_matcher(match);
}

bool GeneParserRule::try_parse(std::string& name, NullableSpliceVariantId& splice_variant_id) {
	match_results<string::const_iterator> results;
	if (regex_match(name, results, matcher_re)) {
		// get splice variant
		splice_variant_id = boost::none;
		if (splice_variant_group) {
			auto variant_str = results.format((make_string() << "$" << *splice_variant_group).str());
			if (!variant_str.empty()) {
				istringstream str(variant_str);
				SpliceVariantId id;
				str >> id;
				ensure(!str.fail(),
						(make_string() << "Error parsing gene variant '" << name << "': '" << variant_str << "' is not a splice variant id").str(),
						ErrorType::GENERIC
				);
				splice_variant_id = id;
			}
		}

		// get gene name
		name = results.format(replace_format);

		return true;
	}
	return false;
}

void GeneParserRule::set_matcher(const std::string& match) {
	matcher = match;
 	matcher_re = boost::regex(matcher, boost::regex::perl | boost::regex::icase);
}

} // end namespace
