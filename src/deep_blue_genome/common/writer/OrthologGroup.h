// Author: Tim Diels <timdiels.m@gmail.com>

/**
 * Writers of ortholog groups
 */

#pragma once

#include <yaml-cpp/yaml.h>

namespace DEEP_BLUE_GENOME {

class OrthologGroup; // TODO all classes in common should be in COMMON namespace

namespace COMMON {
namespace WRITER {

/**
 * Get group as yaml with all its external ids
 */
YAML::Node write_yaml(const OrthologGroup&);

/**
 * Like write_yaml, but with list of genes of the group
 */
YAML::Node write_yaml_with_genes(const OrthologGroup&);

}}} // end namespace
