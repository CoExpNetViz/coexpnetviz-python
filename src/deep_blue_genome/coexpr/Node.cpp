// Author: Tim Diels <timdiels.m@gmail.com>

#include "Node.h"

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

Node::Node()
:	id(next_id++)
{
}

uint64_t Node::get_id() const {
	return id;
}

std::ostream& operator <<(std::ostream& out, const Node& node) {
	out << "n" << node.get_id();
	return out;
}

}} // end namespace
