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
