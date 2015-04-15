// Author: Tim Diels <timdiels.m@gmail.com>

/**
 * Database readers, read data into database
 */

#pragma once

#include <yaml-cpp/yaml.h>

namespace DEEP_BLUE_GENOME {

class Database;

namespace COMMON {
namespace READER {

/**
 * Read ortholog groups from yaml
 */
void read_orthologs_yaml(Database& database, YAML::Node, const std::string& data_root="");

}}} // end namespace
