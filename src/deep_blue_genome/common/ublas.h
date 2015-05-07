/*
 * Copyright (C) 2015 VIB/BEG/UGent - Tim Diels <timdiels.m@gmail.com>
 *
 * This file is part of Deep Blue Genome.
 *
 * Deep Blue Genome is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Deep Blue Genome is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Deep Blue Genome.  If not, see <http://www.gnu.org/licenses/>.
 */

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

// TODO 32 bit row count is sufficient
typedef boost::numeric::ublas::matrix<double> matrix;
typedef boost::numeric::ublas::matrix_indirect<matrix> matrix_indirect;
typedef boost::numeric::ublas::indirect_array<> indirect_array;
typedef DEEP_BLUE_GENOME::indirect_array::array_type array;

}
