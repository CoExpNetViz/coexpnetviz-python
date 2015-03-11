// Author: Tim Diels <timdiels.m@gmail.com>

#include "ErrorType.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

TypedException::TypedException(std::string what, ErrorType error_type)
:	std::runtime_error(what), error_type(error_type)
{
}

int TypedException::get_exit_code() const {
	return static_cast<int>(error_type);
}

ErrorType TypedException::get_type() const {
	return error_type;
}

} // end namespace
