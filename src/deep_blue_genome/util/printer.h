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
 */
#pragma once

#include <deep_blue_genome/util/printer/printer.h>
#include <deep_blue_genome/util/printer/intercalate.h>
