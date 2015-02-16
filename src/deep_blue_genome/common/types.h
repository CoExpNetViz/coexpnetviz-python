// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <mysql++/mysql++.h>

namespace DEEP_BLUE_GENOME {

typedef mysqlpp::sql_bigint_unsigned GeneId;
typedef mysqlpp::sql_bigint_unsigned OrthologGroupId;
typedef mysqlpp::Null<OrthologGroupId> NullableOrthologGroupId;
typedef mysqlpp::sql_int_unsigned GenomeId;
typedef mysqlpp::sql_int_unsigned GeneCollectionId;
typedef mysqlpp::sql_int_unsigned GeneExpressionMatrixId;
typedef GeneExpressionMatrixId ExpressionMatrixId;
typedef mysqlpp::Null<ExpressionMatrixId> NullableGeneExpressionMatrixId;
typedef NullableGeneExpressionMatrixId NullableExpressionMatrixId;
typedef mysqlpp::sql_int_unsigned ClusteringId;
typedef mysqlpp::sql_int_unsigned ClusterId;
typedef mysqlpp::sql_varchar_null NullableGeneWebPage;
typedef mysqlpp::sql_int_unsigned SpliceVariantId;
typedef mysqlpp::Null<SpliceVariantId> NullableSpliceVariantId;
typedef mysqlpp::sql_bigint_unsigned GeneVariantId;
typedef mysqlpp::sql_int_unsigned GeneParserRuleId;
typedef mysqlpp::sql_int RegexGroup;
typedef mysqlpp::Null<RegexGroup> NullableRegexGroup;

} // end namespace
