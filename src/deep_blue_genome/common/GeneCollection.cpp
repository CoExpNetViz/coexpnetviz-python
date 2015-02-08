// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneCollection.h"
#include <boost/regex.hpp>
#include <iomanip>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

GeneCollection::GeneCollection(const std::string& name, const std::string& gene_format_match, const std::string& gene_format_replace, const std::string& gene_web_page, Database& database)
:	name(name),
 	gene_format_match(gene_format_match),
 	gene_format_match_re(gene_format_match, boost::regex::perl | boost::regex::icase),
 	gene_format_replace(gene_format_replace),
 	gene_web_page(gene_web_page),
 	database(database)
{
}

Gene GeneCollection::get_gene_by_name(const std::string& name) {
	Gene out;
	if (try_get_gene_by_name(name, out)) {
		return out;
	}
	else {
		throw NotFoundException("Gene not part of a known gene collection: " + name);
	}
}

bool GeneCollection::try_get_gene_by_name(const std::string& name, Gene& out) {
	if (regex_match(name, gene_format_match_re)) {
		// Format name
		std::string name = regex_replace(name, gene_format_match_re, gene_format_replace);

		// Select existing
		{
			auto query = database.prepare("SELECT id, ortholog_group_id FROM gene WHERE name = %0q");
			query.parse();
			auto result = query.store(name);
			if (result.num_rows() > 0) {
				assert(result.num_rows() == 1);
				auto row = *result.begin();
				out = Gene(row[0], id, row[1], name);
				return true;
			}
		}

		// Else insert as it doesn't exist yet
		{
			auto query = database.prepare("INSERT INTO gene (gene_collection_id, name) VALUES (%0q, %1q)");
			query.parse();
			auto result = query.execute(id, name);
			out = Gene(result.insert_id(), id, mysqlpp::null, name);
			return true;
		}
	}
	return false;
}

std::string GeneCollection::get_name() const {
	return name;
}

bool GeneCollection::has_gene_web_page() const {
	return gene_web_page != "";
}

std::string GeneCollection::get_gene_web_page() const {
	assert(has_gene_web_page());
	return gene_web_page;
}


} // end namespace
