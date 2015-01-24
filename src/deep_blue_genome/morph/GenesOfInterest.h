// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <boost/regex.hpp>
#include <deep_blue_genome/common/GeneMapping.h>

namespace DEEP_BLUE_GENOME {
namespace MORPH {

/**
 * Genes of interest (GOI)
 *
 * A collection of genes.
 */
class GenesOfInterest
{
public:
	/**
	 * @param name Name of goi
	 * @param path Path to plain text TODO describe format
	 */
	GenesOfInterest(std::string name, std::string path, const boost::regex& gene_pattern);
	const std::vector<std::string>& get_genes() const;
	std::string get_name() const;

	/**
	 * Apply mappings to genes
	 */
	void apply_mapping(const DEEP_BLUE_GENOME::GeneMapping&);

private:
	std::string name;
	std::vector<std::string> genes;
};

}}
