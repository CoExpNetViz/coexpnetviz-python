// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/noncopyable.hpp>
#include <unordered_map>
#include <deep_blue_genome/coexpr/BaitGroup.h>

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

// TODO this kind of map with some default construction is a frequent pattern. Should apply some DRY to it
class BaitGroups : public boost::noncopyable
{
public:
	typedef std::unordered_map<std::string, BaitGroup> Groups;

	/**
	 * Get Group of gene
	 */
	BaitGroup& get(std::string name);

	Groups::iterator begin();
	Groups::iterator end();

private:
	Groups groups;
};

}} // end namespace
