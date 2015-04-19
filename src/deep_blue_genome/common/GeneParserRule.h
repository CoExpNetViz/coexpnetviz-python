// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

// TODO separate header
#define BOOST_REGEX_USE_CPP_LOCALE
#include <boost/regex.hpp>

#include <string>
#include <deep_blue_genome/common/Serialization.h>
#include <deep_blue_genome/common/types.h>

namespace DEEP_BLUE_GENOME {

class Database;

#pragma db object
/**
 * Parsing rules for genes of a gene collection
 */
class GeneParserRule {
public:
	GeneParserRule(const std::string& match, const std::string& replace);

	/**
	 * Tries to parse gene name
	 *
	 * If successful, name will be formatted canonically without splicing suffix,
	 * and splicing_variant will contain the respective splicing number. Else
	 * none of the args are changed.
	 *
	 * @param splice_variant Not used as input, will contain the splice number.
	 */
	bool try_parse(std::string& name, NullableSpliceVariantId& splice_variant);

public: // treat as private (failed to friend boost::serialization)
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

	GeneParserRule();

private:
	void set_matcher(const std::string& matcher);

private:
	std::string matcher;
	std::string replace_format;

	boost::regex matcher_re;
};


} // end namespace


/////////////////////////
// hpp

namespace DEEP_BLUE_GENOME {

template<class Archive>
void GeneParserRule::serialize(Archive& ar, const unsigned int version) {
	ar & matcher;
	ar & replace_format;
	if (Archive::is_loading::value) {
		set_matcher(matcher);
	}
}

}
