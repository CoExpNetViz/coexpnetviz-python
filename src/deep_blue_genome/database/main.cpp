// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <deep_blue_genome/common/util.h>
//#include "Application.h"

#include <deep_blue_genome/common/Database.h>

using namespace std;

int main(int argc, char** argv) {
	using namespace DEEP_BLUE_GENOME;
	graceful_main([argc, argv](){
		Database database("/home/limyreth/dbg_db");
		for (auto name : database.get_species_names()) {
			cout << name << "\n";
		}
		//Application app(argc, argv);
		//app.run();
	});
}
