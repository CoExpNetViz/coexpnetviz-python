// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <deep_blue_genome/common/util.h>
#include "Application.h"

using namespace std;

int main(int argc, char** argv) {
	using namespace MORPHC;
	graceful_main([argc, argv](){
		Application app(argc, argv);
		app.run();
	});
}
