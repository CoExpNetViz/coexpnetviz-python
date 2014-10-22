// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <vector>
#include "Gene.h"

class GenesOfInterest
{
public:
	std::vector<Gene*>& get_genes();

private:
	std::string name;
	std::vector<Gene*> genes;
};
