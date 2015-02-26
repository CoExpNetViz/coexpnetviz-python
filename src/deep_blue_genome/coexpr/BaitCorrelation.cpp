// Author: Tim Diels <timdiels.m@gmail.com>

#include "BaitCorrelation.h"

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

BaitCorrelation::BaitCorrelation(const Gene& bait, double correlation)
:	bait(bait), correlation(correlation)
{
}

const Gene& BaitCorrelation::get_bait() const {
	return bait;
}

double BaitCorrelation::get_correlation() const {
	return correlation;
}

}} // end namespace
