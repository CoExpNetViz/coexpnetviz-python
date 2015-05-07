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

#include <string>
#include <stdexcept>

namespace DEEP_BLUE_GENOME {

// Note: when changing enum, be sure to update list in usage printing of Application.cpp
enum class ErrorType : int {
	NONE = 0,
	GENERIC,
	INVALID_GOI_GENE,
	SPLICE_VARIANT_INSTEAD_OF_GENE
};

// XXX in a normal world one'd use inheritance to add types..., derived types...
class TypedException : public std::runtime_error
{
public:
	TypedException(std::string what, ErrorType error_type);

	int get_exit_code() const;
	ErrorType get_type() const;

private:
	ErrorType error_type;
};

} // end namespace
