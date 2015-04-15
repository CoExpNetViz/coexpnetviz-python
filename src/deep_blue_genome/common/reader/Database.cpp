// Author: Tim Diels <timdiels.m@gmail.com>

#include "Database.h"
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/DataFileImport.h>
#include <deep_blue_genome/common/database_all.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COMMON {
namespace READER {

void read_orthologs_yaml(Database& database, YAML::Node orthologs, const std::string& data_root) {
	DataFileImport importer(database);

	for (auto node : orthologs) {
		auto path = prepend_path(data_root, node["path"].as<string>());
		importer.add_orthologs(node["name"].as<string>(), path);
	}
}

}}} // end namespace
