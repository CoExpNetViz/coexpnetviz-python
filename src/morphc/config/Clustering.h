// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>

namespace MORPHC {
namespace CONFIG {

class Clustering {
public:
	Clustering(std::string name, std::string path);

	std::string get_name() const;
	std::string get_path() const;

private:
	std::string name;
	std::string path;
};

}}
