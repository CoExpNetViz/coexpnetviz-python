// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>
#include <vector>
#include <string>

namespace DEEP_BLUE_GENOME {

class Gene;

namespace COEXPR {

class OrthologGroupInfo;
class OrthologGroupInfos;

/**
 * Writes out cytoscape files
 */
class CytoscapeWriter : public boost::noncopyable
{
public:
	void write(std::string install_dir, const std::vector<Gene*>& baits, const std::vector<OrthologGroupInfo*>& neighbours, OrthologGroupInfos* groups);
private:
};


}} // end namespace
