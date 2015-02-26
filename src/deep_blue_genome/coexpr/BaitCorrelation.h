// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

namespace DEEP_BLUE_GENOME {

class Gene;

namespace COEXPR {

/**
 * Correlation to a bait gene
 *
 * Data class of Group.
 */
class BaitCorrelation {
public:
	BaitCorrelation(const Gene& bait, double correlation);

	const Gene& get_bait() const;
	double get_correlation() const;

private:
	const Gene& bait;
	double correlation; // correlation to owner
};

}} // end namespace
