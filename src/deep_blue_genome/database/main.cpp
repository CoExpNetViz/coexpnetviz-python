// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <deep_blue_genome/common/util.h>
#include <deep_blue_genome/common/database_all.h>
#include <boost/program_options.hpp>

using namespace std;
using namespace DEEP_BLUE_GENOME;
//using namespace DEEP_BLUE_GENOME::DATABASE;

// TODO introduce concept of canonical gene names, after input from outside everything should be in its canonical format
int main(int argc, char** argv) {
	graceful_main([argc, argv](){
		// Read args
		namespace po = boost::program_options;

		po::options_description desc("Allowed options");
		desc.add_options()
		    ("help", "produce help message")
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
		Database database;

		if (vm.count("update")) {
			database.update(vm["update"].as<string>());
			return;
		}

		// If no args, then print db contents
		/*for (auto name : database.get_species_names()) {
			cout << name << "\n";
		}*/
	});
}
