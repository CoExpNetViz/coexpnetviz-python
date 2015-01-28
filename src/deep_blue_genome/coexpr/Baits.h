// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

/**
 * A collection of genes, possibly of various species
 */
class Baits // TODO quite similar to GOI in the sense that applying a gene mapping to it is interesting
{
public:
	/**
	 * @param path Path to plain text TODO describe format (same as GOI)
	 */
	Baits(std::string path);
	const std::vector<std::string>& get_genes() const;

private:
	std::vector<std::string> genes;
};

}}
