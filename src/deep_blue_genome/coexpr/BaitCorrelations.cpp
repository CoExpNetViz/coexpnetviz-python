// Author: Tim Diels <timdiels.m@gmail.com>

#include "BaitCorrelations.h"

using namespace std;
using namespace DEEP_BLUE_GENOME;

namespace DEEP_BLUE_GENOME {
namespace COEXPR {

BaitCorrelations::BaitCorrelations(const Gene& bait)
:	bait(bait)
{
}

const Gene& BaitCorrelations::get_bait() const {
	return bait;
}

double BaitCorrelations::get_max_correlation() const {
	double max_ = correlations.front().second;
	for (auto& p : correlations) {
		max_ = max(p.second, max_);
	}
	return max_;
}

void BaitCorrelations::add_correlation(const Gene& target, double correlation) {
	correlations.emplace_back(&target, correlation);
}

}} // end namespace
