// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>

namespace MORPHC {

class GeneDescriptions : public boost::noncopyable {
public:
	GeneDescriptions(std::string path);

	/**
	 * Get description of gene
	 */
	std::string get(std::string gene) const;

private:
	std::unordered_map<std::string, std::string> descriptions; // gene -> description
};

}
