// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <mysql++/mysql++.h>

namespace DEEP_BLUE_GENOME {

typedef mysqlpp::sql_bigint_unsigned GeneId;
typedef mysqlpp::sql_bigint_unsigned OrthologGroupId;
typedef mysqlpp::sql_int_unsigned GenomeId;
typedef mysqlpp::sql_int_unsigned GeneCollectionId;
typedef mysqlpp::sql_int_unsigned ExpressionMatrixId;
typedef mysqlpp::sql_int_unsigned ClusteringId;
typedef mysqlpp::sql_int_unsigned ClusterId;

} // end namespace
