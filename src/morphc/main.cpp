// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include "Application.h"
#include "util.h"

// TODO fiddling with matrix orientation, would it help performance?
// TODO perhaps not all asserts should be disabled at runtime (e.g. input checks may want to remain)

using namespace std;

int main(int argc, char** argv) {
	try {
		MORPHC::Application app(argc, argv);
		app.run();
	}
	catch (const exception& e) {
		cerr << "Exception: " << MORPHC::exception_what(e) << endl;
		return 1;
	}
	return 0;
}
