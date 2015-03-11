// Author: Tim Diels <timdiels.m@gmail.com>

/**
 * Commands that can be executed via CLI
 */

#pragma once

#include <string>

namespace DEEP_BLUE_GENOME {

class Database;

namespace DATABASE {
	/**
	 * Create database with new data, overwrite if it exists
	 *
	 * TODO specify expected yaml format
	 */
	void create(Database&, std::string yaml_path);
}

} // end namespace
