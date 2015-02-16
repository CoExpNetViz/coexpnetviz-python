// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneParserRule.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneParserRule::GeneParserRule(GeneParserRuleId id, Database& database)
:	id(id), database(database)
{
	auto query = database.prepare("SELECT matcher, replace_format, splice_variant_group FROM gene_parser_rule WHERE id = %0q");
	query.parse();
	auto result = query.store(id);

	if (result.num_rows() == 0) {
		throw NotFoundException((make_string() << "GeneParserRule with id " << id << " not found").str());
	}

	assert(result.num_rows() == 1);
	auto row = *result.begin();
	set_matcher(row[0].conv<std::string>(""));
	replace_format = row[1].conv<std::string>("");
	splice_variant_group = row[2].conv<decltype(splice_variant_group)>(mysqlpp::null);
}

GeneParserRule::GeneParserRule(const std::string& match, const std::string& replace, NullableRegexGroup group, Database& database)
:	id(0), replace_format(replace), splice_variant_group(group), database(database)
{
	set_matcher(match);
}

bool GeneParserRule::try_parse(std::string& name, NullableSpliceVariantId& splice_variant_id) {
	match_results<string::const_iterator> results;
	if (regex_match(name, results, matcher_re)) {
		name = results.format(replace_format);
		if (!splice_variant_group.is_null) {
			auto variant_str = results.format((make_string() << "$" << splice_variant_group.data).str());
			istringstream str(variant_str);
			str.exceptions(std::ios::failbit);
			SpliceVariantId id;
			str >> id;
			splice_variant_id = id;
		}
		else {
			splice_variant_id = mysqlpp::null;
		}
		return true;
	}
	return false;
}

void GeneParserRule::set_matcher(const std::string& match) {
	matcher = match;
 	matcher_re = boost::regex(matcher, boost::regex::perl | boost::regex::icase);
}

// TODO look for a db persistence library for C++
void GeneParserRule::database_insert() {
	assert(id == 0);
	auto query = database.prepare("INSERT INTO gene_parser_rule(matcher, replace_format, splice_variant_group) VALUES (%0q, %1q, %2q)");
	query.parse();
	auto result = query.execute(matcher, replace_format, splice_variant_group);
	id = result.insert_id();
}

} // end namespace
