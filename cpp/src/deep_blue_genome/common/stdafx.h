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

/**
 * Includes to all used third party headers, saves us the bother of manually including them and this header is precompiled
 */

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <string>
#include <exception>
#include <stdexcept>
#include <utility>
#include <functional>
#include <memory>
#include <random>
#include <errno.h>

#include <map>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <list>

#include <yaml-cpp/yaml.h>

#include <gsl/gsl_statistics_double.h>

#include <boost/archive/archive_exception.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/program_options.hpp>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/set.hpp>

#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>

#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/join.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>

#include <boost/spirit/include/qi.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

#include <boost/filesystem.hpp>

#define BOOST_REGEX_USE_CPP_LOCALE
#include <boost/regex.hpp>

#include "ublas.h"
