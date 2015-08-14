// Author: Tim Diels <timdiels.m@gmail.com>

#include <deep_blue_genome/common/stdafx.h>
#include "util.h"

using namespace std;

namespace DEEP_BLUE_GENOME {

void read_file(std::string path, std::function<const char* (const char*, const char*)> reader) {
	try {
		boost::iostreams::mapped_file mmap(path, boost::iostreams::mapped_file::readonly);
		auto begin = mmap.const_data();
		auto end = begin + mmap.size();

		// trim last newline if any
		if (begin != end && *(end-1) == '\n') {
			end--;
			if (begin != end && *(end-1) == '\r')
				end--;
		}

		const char* current = nullptr;
		try {
			current = reader(begin, end);
		}
		catch (const boost::spirit::qi::expectation_failure<const char*>& e) {
			throw runtime_error(exception_what(e)); // get error message here, as outside the outer try block it'd segfault when reading from file (which this one does)
		}

		current = find_if_not(current, end, [](char c){ return isspace(c); }); // skip trailing whitespace
		if (current != end) {
			ostringstream out;
			out << "Trailing characters at end of file: '";
			copy(current, end, ostream_iterator<char>(out));
			out << "'";
			throw runtime_error(out.str());
		}
	}
	catch (const exception& e) {
		throw runtime_error((make_string() << "Error while reading '" << path << "': " << exception_what(e)).str());
	}
}

string prepend_path(const string& prefix, const string& path) {
	if (prefix.empty())
		return path;

	if (path.at(0) == '/')
		return path;
	else
		return prefix + "/" + path;
}

void ensure(bool condition, std::string error_message, ErrorType error_category) {
	if (!condition)
		throw TypedException(error_message, error_category);
}

/**
 * Prints something of boost spirit
 *
 * Source: http://www.boost.org/doc/libs/1_57_0/libs/spirit/doc/html/spirit/qi/reference/basics.html#spirit.qi.reference.basics.examples
 */
class SpiritPrinter
{
public:
    typedef boost::spirit::utf8_string string;

    SpiritPrinter(ostream& out)
    :	out(out)
    {
    }

    void element(string const& tag, string const& value, int depth) const
    {
        for (int i = 0; i < (depth*4); ++i) // indent to depth
            out << ' ';

        out << "tag: " << tag;
        if (value != "")
            out << ", value: " << value;
        out << "\n";
    }

private:
    ostream& out;
};

std::string exception_what(const exception& e) {
	{
		auto expectation_failure_ex = dynamic_cast<const boost::spirit::qi::expectation_failure<const char*>*>(&e);
		if (expectation_failure_ex) {
			auto& e = *expectation_failure_ex;
			ostringstream str;
			str << e.what() << ": \n";

			str << "Expected: \n";
			SpiritPrinter printer(str);
			boost::spirit::basic_info_walker<SpiritPrinter> walker(printer, e.what_.tag, 0);
			boost::apply_visitor(walker, e.what_.value);

			str << "Got: \"" << std::string(e.first, e.last) << '"' << endl;

			return str.str();
		}
	}

	{
		auto ios_clear_ex = dynamic_cast<const std::ios::failure*>(&e);
		if (ios_clear_ex) {
			return (make_string() << ios_clear_ex->what() << ": " << strerror(errno)).str();
		}
	}

	{
		auto archive_ex = dynamic_cast<const boost::archive::archive_exception*>(&e);
		if (archive_ex) {
			return (make_string() << archive_ex->what() << ": " << strerror(errno)).str();
		}
	}

	return e.what();
}

std::string& to_lower(std::string& data) {
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	return data;
}

void graceful_main(std::function<void()> fragile_main) {
	try {
		fragile_main();
	}
	catch (const TypedException& e) {
		cerr << "Exception: " << e.what() << endl;
		exit(e.get_exit_code());
	}
	catch (const exception& e) {
		cerr << "Exception: " << exception_what(e) << endl;
		exit(static_cast<int>(ErrorType::GENERIC));
	}

	exit(EXIT_SUCCESS);
}

template <typename T>
auto as_printable_integer(T i) -> decltype(+i)
{
  return +i;
}

std::string to_file_name(const std::string& in) {
	std::stringstream out;
	for (auto c : in) {
		switch(c) {
		case '%':
		case '<':
		case '>':
		case ':':
		case '"':
		case '/':
		case '\\':
		case '|':
		case '?':
		case '*':
			out << "%" << std::hex << as_printable_integer(c); // %XX, (+c prints the char as a number: http://www.parashift.com/c++-faq/print-char-or-ptr-as-number.html)
			break;

		default:
			out << c;
			break;
		}
	}
	return out.str();
}


} // end namespace
