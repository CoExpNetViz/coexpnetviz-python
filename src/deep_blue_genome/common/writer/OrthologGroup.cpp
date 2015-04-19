// Author: Tim Diels <timdiels.m@gmail.com>

#include "OrthologGroup.h"
#include <deep_blue_genome/common/Gene.h>
#include <deep_blue_genome/common/OrthologGroup.h>

using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COMMON {
namespace WRITER {

YAML::Node write_yaml(const OrthologGroup& group) {
	YAML::Node node;

	for (auto& family_id : group.get_external_ids()) {
		YAML::Node family;
		family["source"] = family_id.get_source();
		family["id"] = family_id.get_id();
		node.push_back(family);
	}

	return node;
}


YAML::Node write_yaml_with_genes(const OrthologGroup& group) {
	YAML::Node node;
	node["external_ids"] = write_yaml(group);
	YAML::Node genes; // TODO can this be initialised to an empty list so we don't get ~ on output?
	for (auto gene : group.get_genes()) {
		genes.push_back(gene->get_name());
	}
	node["genes"] = genes;
	return node;
}

}}} // end namespace
