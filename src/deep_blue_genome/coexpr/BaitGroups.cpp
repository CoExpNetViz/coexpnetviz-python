// Author: Tim Diels <timdiels.m@gmail.com>

#include "BaitGroups.h"

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

BaitGroup& BaitGroups::get(std::string name) {
	auto it = groups.find(name);
	if (it == groups.end()) {
		return groups.emplace(piecewise_construct,
				forward_as_tuple(name),
				forward_as_tuple(name)).first->second;
	}
	else {
		return it->second;
	}
}

BaitGroups::Groups::iterator BaitGroups::begin() {
	return groups.begin();
}

BaitGroups::Groups::iterator BaitGroups::end() {
	return groups.end();
}

}} // end namespace
