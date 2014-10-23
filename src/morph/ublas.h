// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/numeric/ublas/io.hpp>

typedef boost::numeric::ublas::matrix<double> matrix;
typedef boost::numeric::ublas::matrix_indirect<matrix> matrix_indirect;
typedef matrix::size_type size_type;
//typedef boost::numeric::ublas::matrix<double>::array_type array_type;
typedef boost::numeric::ublas::indirect_array<> indirect_array;
typedef ::indirect_array::array_type array;
