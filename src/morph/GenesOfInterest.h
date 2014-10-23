// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include "Gene.h"

class GenesOfInterest
{
public:
	GenesOfInterest(std::string path);
	const std::vector<std::string>& get_genes();

private:
	std::string name;
	std::vector<std::string> genes;
};
