// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneMapping.h"

using namespace std;

namespace MORPHC {

GeneMapping::GeneMapping(string path)
:	mapping(path)
{
}

std::string GeneMapping::get(std::string gene) const {
	return mapping.get().at(gene);
}

bool GeneMapping::has(std::string gene) const {
	return mapping.get().find(gene) != mapping.get().end();
}

}
