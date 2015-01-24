// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <deep_blue_genome/common/util.h>
#include "Application.h"

// TODO fiddling with matrix orientation, would it help performance?
// TODO perhaps not all asserts should be disabled at runtime (e.g. input checks may want to remain)

/**
 * General notes about the code:
 * - goi := genes of interest (always plural, thus not: gene of interest)
 * - gois := goi groups = a collection of goi ~= vector<vector<gene of interest>>
 */

using namespace std;

// TODO lots of validation on input when read for first time (i.e. reading from plain text), then when loaded from binary no need for validation
int main(int argc, char** argv) {
	using namespace DEEP_BLUE_GENOME;
	using namespace DEEP_BLUE_GENOME::MORPH;
	graceful_main([argc, argv](){
		Application app(argc, argv);
		app.run();
	});
}
