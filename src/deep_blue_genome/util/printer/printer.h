// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

namespace DEEP_BLUE_GENOME {

class Printer {
public:
	Printer(std::function<void(std::ostream&)> printer_func)
	:	print(printer_func)
	{
	}

private:
	friend std::ostream& operator<<(std::ostream& out, const Printer& obj) {
		obj.print(out);
		return out;
	}

private:
	std::function<void(std::ostream&)> print;
};

/**
 * Prints to stream using provided function
 *
 * @param printer_func Function with signature void(ostream&)
 */
Printer make_printer(std::function<void(std::ostream&)> printer_func) {
	return Printer(printer_func);
}

} // end namespace
