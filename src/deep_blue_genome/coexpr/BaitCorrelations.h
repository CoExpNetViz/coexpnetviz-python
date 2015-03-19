// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <vector>
#include <utility>

namespace DEEP_BLUE_GENOME {

class Gene;

namespace COEXPR {

/**
 * Correlations to a bait gene
 *
 * Data class of Group.
 */
class BaitCorrelations {
public:
	BaitCorrelations(const Gene& bait);

	const Gene& get_bait() const;
	double get_max_correlation() const;
	void add_correlation(const Gene& target, double correlation);

private:
	const Gene& bait;
	std::vector<std::pair<const Gene*, double>> correlations; // correlation between target and bait
};

}} // end namespace
