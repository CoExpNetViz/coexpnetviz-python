// Author: Tim Diels <timdiels.m@gmail.com>

#include "Clustering.h"

using namespace std;

namespace MORPHC {
namespace CONFIG {

Clustering::Clustering(string name, string path)
:	name(name), path(path)
{
}

string Clustering::get_name() const {
	return name;
}

string Clustering::get_path() const {
	return path;
}

}}
