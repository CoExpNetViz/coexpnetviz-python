// Author: Tim Diels <timdiels.m@gmail.com>

/**
 * Commands that can be executed via CLI
 */

#pragma once

#include <string>

namespace DEEP_BLUE_GENOME {

class Database;

namespace DATABASE {

	namespace impl {
		/**
		 * Update database
		 *
		 * TODO specify expected yaml format
		 *
		 * @param create If true, database will be created if it doesn't exist and overwritten if it does exist.
		 */
		void database_update(bool create, const std::string& database_path, const std::string& yaml_path);
	}

	void database_create(const std::string& database_path, const std::string& yaml_path);

	void database_add(const std::string& database_path, const std::string& yaml_path);

	void database_dump(const std::string& database_path, const std::string& dump_path);

	void database_verify(const std::string& database_path);
}

} // end namespace
