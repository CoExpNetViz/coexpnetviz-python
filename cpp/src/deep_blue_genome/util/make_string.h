#pragma once

#include <string>
#include <sstream>

namespace DEEP_BLUE_GENOME {

// This class taken from: http://stackoverflow.com/a/25351759/1031434
// Contributed by Jason R
class make_string
{
public:
	make_string()
	{
		oss.exceptions(std::ios::failbit);
	}

    template <typename T>
    explicit make_string(T && rhs)
    {
        oss << rhs;
    }

    template <typename T>
    make_string &operator<<(T && rhs)
    {
        oss << rhs;
        return *this;
    }

    operator std::string() const
    {
        return oss.str();
    }

    std::string str() const
    {
        return oss.str();
    }

private:
    std::ostringstream oss;
};

} // end namespace
