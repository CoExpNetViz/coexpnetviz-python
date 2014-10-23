// Author: Tim Diels <timdiels.m@gmail.com>

#include "GenesOfInterest.h"
#include "util.h"

using namespace std;

GenesOfInterest::GenesOfInterest(std::string path)
:	name(path)
{
	// load
	read_file(path, [this](ifstream& in) {
		while (in.good()) {
			string gene_name;
			in >> gene_name;
			if (in.fail()) {
				break;
			}
			genes.push_back(gene_name);
		}
	});
}
const vector<std::string>& GenesOfInterest::get_genes() {
	return genes;
}
