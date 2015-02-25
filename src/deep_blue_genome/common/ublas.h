// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#define BOOST_UBLAS_NDEBUG  // comment this line for massive ublas assertions (slows down debug a lot. You should do it at least once after changing matrix/vector computations though, with a very small joblist + 1 dataset + 1clusterings)
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace DEEP_BLUE_GENOME {

// TODO 32 bit row count is sufficient, but perhaps 64 bit works best on 64 bit systems. Not sure what the default is
typedef boost::numeric::ublas::matrix<double> matrix;
typedef boost::numeric::ublas::matrix_indirect<matrix> matrix_indirect;
typedef boost::numeric::ublas::indirect_array<> indirect_array;
typedef DEEP_BLUE_GENOME::indirect_array::array_type array;

}
