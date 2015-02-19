// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <boost/regex.hpp>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

class Database;

#pragma db object
/**
 * Parsing rules for genes of a gene collection
 */
class GeneParserRule {
public:
	GeneParserRule(GeneParserRuleId, Database&);
	GeneParserRule(const std::string& match, const std::string& replace, NullableRegexGroup, Database&);

	void database_insert();

	/**
	 * Tries to parse gene name
	 *
	 * If successful, name will be formatted canonically without splicing suffix,
	 * and splicing_variant will contain the respective splicing number. Else
	 * none of the args are changed.
	 *
	 * @param splicing_variant Not used as input, will contain the splicing number.
	 */
	bool try_parse(std::string& name, NullableSpliceVariantId& splicing_variant);

private:
	void set_matcher(const std::string& matcher);

private:
	friend class odb::access;

	GeneParserRule() {};  // for ODB

	#pragma db id auto
	GeneParserRuleId id; // TODO Much later: can totally place ids inline, given that elsewhere we don't actually use them...

	std::string matcher; // See create_db.sql for meaning of these fields
	std::string replace_format;
	NullableRegexGroup splice_variant_group;

	#pragma db transient
	boost::regex matcher_re;
};


} // end namespace
