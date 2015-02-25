// Author: Tim Diels <timdiels.m@gmail.com>

#include "GeneVariant.h"
#include <boost/algorithm/string.hpp>


using namespace std;

namespace DEEP_BLUE_GENOME {

GeneVariant::GeneVariant()
:	dna_sequence(*this)
{
}

GeneVariant::~GeneVariant()
{
}

DNASequence& GeneVariant::get_dna_sequence() {
	return dna_sequence;
}

void GeneVariant::set_functional_annotation(std::string annotation) {
	boost::algorithm::trim(annotation);
	assert(!annotation.empty());
	functional_annotation = annotation;
}

std::ostream& operator<<(std::ostream& str, const GeneVariant& variant) {
	variant.print(str);
	return str;
}

} // end namespace
