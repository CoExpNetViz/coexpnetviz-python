// Author: Tim Diels <timdiels.m@gmail.com>

#include "Species.h"
#include <boost/regex.hpp>
#include <iomanip>
#include <boost/iterator/transform_iterator.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>

using namespace std;
using namespace boost;

namespace DEEP_BLUE_GENOME {

Species::Species(std::string name, Database& database)
:	name(name), database(database), has_gene_descriptions_(false), has_canonical_mapping_(false)
{
}

void Species::set_gene_pattern(std::string gene_pattern) {
	this->gene_pattern = gene_pattern;
	this->gene_pattern_re = boost::regex(gene_pattern, boost::regex::perl|boost::regex::icase);
}

std::string Species::get_name() const {
	return name;
}

Iterable<Species::name_iterator> Species::get_gene_expression_matrices() const {
	return make_iterable(gene_expression_matrix_names.begin(), gene_expression_matrix_names.end());
}

Iterable<Species::name_iterator> Species::get_clusterings(std::string gene_expression_matrix) const {
	auto& specific_clusterings = gene_expression_matrices.find(gene_expression_matrix)->second;
	return make_iterable(specific_clusterings.begin(), specific_clusterings.end());
}

const std::string& Species::get_gene_pattern() const {
	return gene_pattern;
}

const boost::regex& Species::get_gene_pattern_re() const {
	return gene_pattern_re;
}

bool Species::has_gene_web_page() const {
	return gene_web_page != "";
}

void Species::set_gene_web_page(const std::string web_page) {
	gene_web_page = web_page;
}

const std::string& Species::get_gene_web_page() const {
	assert(has_gene_web_page());
	return gene_web_page;
}

bool Species::has_gene_descriptions() const {
	return has_gene_descriptions_;
}

void Species::has_gene_descriptions(bool has_it) {
	has_gene_descriptions_ = has_it;
}

std::shared_ptr<GeneDescriptions> Species::get_gene_descriptions() const {
	return database.get_gene_descriptions(name);
}

bool Species::has_canonical_mapping() const {
	return has_canonical_mapping_;
}

void Species::has_canonical_mapping(bool has_it) {
	has_canonical_mapping_ = has_it;
}

std::shared_ptr<GeneMapping> Species::get_canonical_mapping() const {
	return database.get_canonical_mapping(name);
}

std::shared_ptr<GeneExpressionMatrix> Species::get_gene_expression_matrix(std::string matrix) const {
	return database.get_gene_expression_matrix(name, matrix);
}

std::shared_ptr<GeneMapping> Species::get_ortholog_mapping(std::string species) const {
	return database.get_ortholog_mapping(GeneMappingId(name, species));
}

}
