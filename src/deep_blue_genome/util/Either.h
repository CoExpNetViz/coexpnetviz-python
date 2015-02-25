// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

namespace DEEP_BLUE_GENOME {

template<class First, class Second>
class Either
{
public:
	Either(First first)
	:	first(first), has_first_(true)
	{
	}

	Either(Second second)
	:	second(second), has_first_(false)
	{
	}

	bool has_first() const {
		return has_first_;
	}

	bool has_second() const {
		return !has_first_;
	}

	const First& get_first() const {
		assert(has_first_);
		return first;
	}

	const Second& get_second() const {
		assert(!has_first_);
		return second;
	}

private:
	union {
		First first;
		Second second;
	};
	bool has_first_;
};

} // end namespace
