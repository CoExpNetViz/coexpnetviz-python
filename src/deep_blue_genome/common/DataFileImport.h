/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

namespace DEEP_BLUE_GENOME {

class Database;
class GeneExpressionMatrix;

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
	 * - line format (perl regex style): ortho_group_id(\tgene){2,}
	 *
	 * Each ortho_group_id may appear only once.
	 *
	 * If 2 groups overlap, they are merged with a warning.
	 */
	void add_orthologs(std::string source_name, std::string path);

	/**
	 * Add expression matrix from a TODO particular plain text format
	 *
	 * @return the added matrix
	 */
	GeneExpressionMatrix& add_gene_expression_matrix(const std::string& name, const std::string& path);

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
