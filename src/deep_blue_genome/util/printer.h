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

/**
 * An alternative to boost::spirit::karma
 *
 * Why not karma?
 * - Ease of use: usage errors lead to hard to understand compile errors; online tutorial covers only the very basics.
 *
 * Main aims of this 'library':
 * - Easy to use: lambda magic allowed, but no hardcore template metaprogramming
 * - Stream, iterator support without much copying
 *
 * Results:
 * - Due to limitations of GCC 4.8 (bug with lambda capture of variadic args, and lack of c++14 features),
 *   the lib uses intermediate strings.
 * - Once GCC has c++14, the lib can be upgraded to stream instead of using intermediate strings,
 *   without breaking the current interface.
 */
#pragma once

#include <deep_blue_genome/util/printer/intercalate.h>
#include <deep_blue_genome/util/printer/printer.h>
//TODO rename to writer?
