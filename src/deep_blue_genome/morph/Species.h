// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <deep_blue_genome/common/Species.h>

namespace DEEP_BLUE_GENOME {

class Database;

namespace MORPH {

/**
 * A species
 */
class Species {
public:
	Species(std::string name, Database&);

	void add_goi(std::string name, std::string path);

	/**
	 * Find best ranking for each goi, save best rankings in output_path, save at most top_k genes of each ranking
	 */
	void run_jobs(std::string output_path, int top_k, bool output_yaml);

	std::string get_name() const;

private:
	std::shared_ptr<DEEP_BLUE_GENOME::Species> species;
	Database& database;
	std::vector<std::pair<std::string, std::string>> gois;
};

}}
