// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <string>
#include <unordered_map>
#include <boost/noncopyable.hpp>

namespace DEEP_BLUE_GENOME {

/**
 * Functional annotations
 */
class GeneDescriptions : public boost::noncopyable {
public:
	GeneDescriptions(std::string path);

	/**
	 * Construct invalid object for serialization
	 */
	GeneDescriptions() {}

	/**
	 * Get description of gene
	 */
	std::string get(std::string gene) const;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version);

private:
	std::unordered_map<std::string, std::string> mapping;; // gene -> description
};


/////////////////////
// hpp

template<class Archive>
void GeneDescriptions::serialize(Archive& ar, const unsigned int version) {
	ar & mapping;
}

}
