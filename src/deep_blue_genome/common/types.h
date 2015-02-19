// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <odb/nullable.hxx>

namespace DEEP_BLUE_GENOME {

// TODO rm unused
typedef uint64_t GeneId;
typedef uint64_t OrthologGroupId;
typedef odb::nullable<OrthologGroupId> NullableOrthologGroupId;
typedef uint32_t GenomeId;
typedef uint32_t GeneCollectionId;
typedef uint32_t GeneExpressionMatrixId;
typedef odb::nullable<GeneExpressionMatrixId> NullableGeneExpressionMatrixId;
typedef uint32_t ClusteringId;
typedef uint32_t ClusterId;
typedef odb::nullable<std::string> NullableGeneWebPage;
typedef uint32_t SpliceVariantId;
typedef odb::nullable<SpliceVariantId> NullableSpliceVariantId;
typedef uint64_t GeneVariantId;
typedef uint32_t GeneParserRuleId;
typedef uint32_t RegexGroup;
typedef odb::nullable<RegexGroup> NullableRegexGroup;

} // end namespace
