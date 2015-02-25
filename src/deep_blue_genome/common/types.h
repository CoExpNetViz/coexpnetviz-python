// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <deep_blue_genome/common/ublas.h>

namespace DEEP_BLUE_GENOME {

typedef uint32_t RegexGroup;
typedef boost::optional<RegexGroup> NullableRegexGroup;

typedef uint32_t SpliceVariantId;
typedef boost::optional<SpliceVariantId> NullableSpliceVariantId;

typedef boost::optional<std::string> NullableGeneWebPage;

typedef array::value_type GeneExpressionMatrixRow;



} // end namespace
