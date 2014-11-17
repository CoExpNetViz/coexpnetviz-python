// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <iomanip>
#include "Application.h"

// TODO fiddling with matrix orientation, would it help performance?
// TODO perhaps not all asserts should be disabled at runtime (e.g. input checks may want to remain)

using namespace std;

int main(int argc, char** argv) {
	cout << setprecision(9);

	if (argc != 2) {
		cerr << "Invalid argument count" << endl << endl;
		cerr << "USAGE: morphc path/to/joblist.yaml" << endl;
		return 2;
	}

	string job_list_path(argv[1]);

	MORPHC::Application app(job_list_path);
	app.run();
	return 0;
}
