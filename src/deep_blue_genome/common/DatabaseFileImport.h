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
 * Loaded names are matched case-insensitively with
 */
class DatabaseFileImport {
public:
	/**
	 * Expected format:
	 * - plain text
	 * - tab-separated, columns = src gene\tdst gene
	 */
	static void add_gene_mappings(const std::string& path, Database&);

	static void add_functional_annotations(const std::string& path, Database&);

	/**
	 * Add orthologs, genes of unknown collections are skipped
	 *
	 * Expected format:
	 * - plain text
	 * - Each line is a mapping from one gene to multiple genes, all of which are orthologs of the gene
	 * - line format (regex style): gene\tgene(,gene)*
	 */
	static void add_orthologs(const std::string& path, Database&);

private:
	DatabaseFileImport();
};

} // end namespace
