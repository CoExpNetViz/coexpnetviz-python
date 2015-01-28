// Author: Tim Diels <timdiels.m@gmail.com>

/**
 * Commands that can be executed via CLI
 */

#pragma once

namespace DEEP_BLUE_GENOME {

namespace COMMON {
	class Database;
}

namespace DATABASE {

/**
 * Add data from plaza orthologs file to database
 *
 * Expected format:
 * - plain text
 * - Each line is a mapping from one gene to multiple genes, all of which are orthologs of the gene
 * - line format (regex style): gene\tgene(,gene)*
 *
 * Space complexity to run algorithm: roughly 2*file_size
 *
 * @param path Path to plaza orthologs file
 */
void load_plaza_orthologs(Database& database, std::string path);

}} // end namespace
