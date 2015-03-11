// Author: Tim Diels <timdiels.m@gmail.com>

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
