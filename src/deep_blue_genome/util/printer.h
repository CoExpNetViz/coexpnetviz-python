// Author: Tim Diels <timdiels.m@gmail.com>

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
