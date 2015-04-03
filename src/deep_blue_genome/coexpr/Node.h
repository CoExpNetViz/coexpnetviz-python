// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <iostream>

namespace DEEP_BLUE_GENOME {

namespace COEXPR {

class Node
{
public:
	Node();

	uint64_t get_id() const;

private:
	uint64_t id;
	static uint64_t next_id;
};

std::ostream& operator <<(std::ostream& out, const Node& node);


}} // end namespace
