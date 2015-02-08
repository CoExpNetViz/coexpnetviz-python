// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>

namespace DEEP_BLUE_GENOME {

class Database;

/**
 * Functions for importing data from files to database
 *
 * This class not intended to be used directly, use Database instead.
 *
 * TODO move doc GeneMapping:
 *
 * Maps gene names to other gene names (for mapping between different naming schemes)
 *
 * Format of gene mapping file: tab-separated, first column = src gene, each other columns is a dst gene (for src -> dst mapping)
 *
 * Gene names are matched in a case-insensitive manner.
 */
class DatabaseFileImport {
public:
	static void add_gene_mappings(const std::string& path, Database&);
	static void add_functional_annotations(const std::string& path, Database&);

	/**
	 * Add orthologs, genes of unknown collections are skipped
	 */
	static void add_orthologs(const std::string& path, Database&);

private:
	DatabaseFileImport();
};

} // end namespace
