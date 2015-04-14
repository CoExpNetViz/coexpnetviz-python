// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <boost/program_options.hpp>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/database_all.h>
#include <deep_blue_genome/database/commands.h>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using namespace DEEP_BLUE_GENOME::DATABASE;

int main(int argc, char** argv) {
	graceful_main([argc, argv](){
		// Read args
		namespace po = boost::program_options;

		po::options_description desc("Allowed options");
		desc.add_options()
		    ("help", "produce help message")
		    ("create", po::value<string>(), "Create database using a yaml update description file")
			("add", po::value<string>(), "Add to database using a yaml update description file. Does not overwrite data upon name collision.")
		    ("database-path", po::value<string>(), "Path to directory where database is or should be stored")
		;

		po::positional_options_description positional;
		positional.add("database-path", 1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
		          options(desc).positional(positional).run(), vm);
		po::notify(vm);

		// Execute given command
		string db_path = vm["database-path"].as<string>();

		if (vm.count("create")) {
			database_create(db_path, vm["create"].as<string>());
			return;
		}
		else if (vm.count("add")) {
			database_add(db_path, vm["add"].as<string>());
		}
		else {
			cout << desc << "\n";
		}

		// TODO command that print some of db contents as yaml (for morph); it's a list of gene collections, really
		/*for (auto name : database.get_species_names()) {
			cout << name << "\n";
		}*/
	});
}
