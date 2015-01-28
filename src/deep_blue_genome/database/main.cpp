// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/Database.h>
#include <deep_blue_genome/database/commands.h>
#include <boost/program_options.hpp>

using namespace std;
using namespace DEEP_BLUE_GENOME;
using namespace DEEP_BLUE_GENOME::DATABASE;

// TODO introduce concept of canonical gene names, after input from outside everything should be in its canonical format
int main(int argc, char** argv) {
	// TODO use http://www.boost.org/doc/libs/1_57_0/doc/html/program_options.html
	// -> Loads it into the db by splitting by species, ignoring unknown species with try catch but printing warning. No need to define mapping both ways, plaza file already has both directions.
	//    TODO what's a good data format? species -> species? Or species -> all_species grouped by species. For those orthologs. It would take 150M to load all species, for now... which is easiest, but probably wasteful since you only need but a small (<10) subset of all species. Meh, not sure how to optimise, would pick separate map file for each
	graceful_main([argc, argv](){
		// Read args
		namespace po = boost::program_options;

		po::options_description desc("Allowed options");
		desc.add_options()
		    ("help", "produce help message")
		    ("load-plaza-orthologs", po::value<string>(), "Load plaza orthologs file")
		    ("update", po::value<string>(), "Update using a yaml update description file")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
		    cout << desc << "\n";
		    return;
		}

		// Execute given command
		Database database("/home/limyreth/dbg_db");

		if (vm.count("load-plaza-orthologs")) {
			load_plaza_orthologs(database, vm["load-plaza-orthologs"].as<string>());
			return;
		}

		if (vm.count("update")) {
			database.update(vm["update"].as<string>());
			return;
		}

		// If no args, then print db contents
		for (auto name : database.get_species_names()) {
			cout << name << "\n";
		}
	});
}
