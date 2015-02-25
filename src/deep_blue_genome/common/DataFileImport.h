// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>

namespace DEEP_BLUE_GENOME {

class Database;

/**
 * Functions for importing data files to database
 *
 * This class not intended to be used directly, use Database instead.
 *
 * Loaded names are matched case-insensitively with.
 *
 * Tightly coupled to many classes.
 */
class DataFileImport {
public:
	/**
	 * @param db The data base to import to
	 */
	DataFileImport(Database& db);

	/**
	 * Expected format:
	 * - plain text
	 * - tab-separated, columns = src gene\tdst gene
	 */
	void add_gene_mappings(const std::string& path);

	/**
	 * Add functional annotations to genes
	 *
	 * Silently overwrites current annotation of a gene, even if it already has one
	 */
	void add_functional_annotations(const std::string& path);

	/**
	 * Add orthologs, genes of unknown collections are skipped
	 *
	 * Expected format:
	 * - plain text
	 * - Each line is a mapping from one gene to multiple genes, all of which are orthologs of the gene
	 * - line format (regex style): ortho_group_id\tgene\tgene(,gene)*
	 *
	 * Each ortho_group_id may appear only once.
	 *
	 * If 2 groups overlap, they are merged with a warning.
	 */
	void add_orthologs(const std::string& path);

	/**
	 * Add expression matrix from a TODO particular plain text format
	 */
	void add_gene_expression_matrix(const std::string& name, const std::string& path);

	/**
	 * Add clustering from plain text file
	 *
	 * TODO describe plain text format
	 *
	 * @param expression_matrix If it can only be used with a specific matrix, specify it here, otherwise pass "".
	 */
	void add_clustering(const std::string& name, const std::string& path, const std::string& expression_matrix_name);

private:
	Database& database;
};

} // end namespace
