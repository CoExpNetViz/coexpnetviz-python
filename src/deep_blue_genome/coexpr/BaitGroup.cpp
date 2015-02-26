// Author: Tim Diels <timdiels.m@gmail.com>

#include "BaitGroup.h"

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

BaitGroup::BaitGroup(std::string name)
:	name(std::move(name))
{
}

void BaitGroup::set_colour(std::string colour) {
	this->colour = colour;
}

string BaitGroup::get_colour() const {
	return colour;
}

}} // end namespace
