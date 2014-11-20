// Author: Tim Diels <timdiels.m@gmail.com>

#include <iostream>
#include <iomanip>
#include "Application.h"

// TODO fiddling with matrix orientation, would it help performance?
// TODO perhaps not all asserts should be disabled at runtime (e.g. input checks may want to remain)

using namespace std;

int main(int argc, char** argv) {
	cout << setprecision(9);

	if (argc != 3) {
		cerr << "Invalid argument count" << endl << endl;
		cerr << "USAGE: morphc path/to/joblist.yaml path/to/output_directory" << endl;
		return 1;
	}

	string job_list_path(argv[1]);
	string output_path(argv[2]);

	MORPHC::Application app(job_list_path, output_path);
	app.run();
	return 0;
}
