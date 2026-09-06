//
// MIT License
//
// © ESI Group, 2015
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <pvcop/types/datetime_us.h>

#include <boost/date_time/gregorian/gregorian.hpp>

#include <chrono>
#include <ctime>
#ifdef _WIN32
#include <stdlib.h> // _putenv_s for Windows
#endif

/**
 * We need to tweak the parameters a little bit to allow both
 * "%S.%F" (resp "%S.%f") and "%S%F" (resp "%S%f") to work
 */
static std::string sanitize(const char* params)
{
	const char* map[][2] = {{".%F", "%F"}, {".%f", "%f"}};

	std::string str = params;

	for (const auto& token : map) {
		const std::string from = token[0];
		const std::string to = token[1];

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}

	return str;
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_us::formatter_datetime_us
 *****************************************************************************/

pvcop::types::formatter_datetime_us::formatter_datetime_us(const char* params)
    : formatter<boost::posix_time::ptime>(params)
{
	setlocale(LC_ALL, "C");
#ifdef _WIN32
	_putenv_s("LANG", "C");
	_putenv_s("TZ", "GMT");
#else
	setenv("LANG", "C", 1);
	setenv("TZ", "GMT", 1);
#endif
	tzset();

	set_parameters(params);
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_us::extract_struct
 *****************************************************************************/

const char* pvcop::types::formatter_datetime_us::extract_struct(const char* str, char* buffer) const
{
	size_t len = 0;
	size_t struct_len = 0;

	for (char c = str[len]; c != '\0'; c = str[len++]) {
		if (c == 'T') {
			c = ' ';
		}
		if (not std::isalnum(c) and c != '%') {
			buffer[struct_len++] = c;
		}
	}
	buffer[struct_len] = '\0';

	return buffer;
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_us::extract_struct
 *****************************************************************************/

void pvcop::types::formatter_datetime_us::set_parameters(const char* parameters)
{
	_input_facet = new boost::posix_time::time_input_facet(sanitize(parameters).c_str());
	_input_locale_facet = std::locale(std::locale::classic(), _input_facet);
	_output_facet = new boost::posix_time::time_facet(parameters);
	_output_locale_facet = std::locale(std::locale::classic(), _output_facet);

	formatter<boost::posix_time::ptime>::set_parameters(parameters);
	_is_year_specified = strstr(parameters, "%y") or strstr(parameters, "%Y");
	extract_struct(parameters, _params_struct);
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_us::convert_from_string
 *****************************************************************************/

bool pvcop::types::formatter_datetime_us::convert_from_string(const char* str,
                                                              reference value,
                                                              bool* /*pass_autodetect*/) const
{
	if (strlen(str) > MAX_STRING_LENGTH) {
		return false;
	}

	static const size_t current_year = boost::gregorian::day_clock::universal_day().year();

	char str_struct[MAX_STRING_LENGTH];
	if (strcmp(extract_struct(str, str_struct), _params_struct) != 0) {
		return false;
	}

	std::istringstream date_stream(str);
	cal c{{boost::posix_time::ptime()}};
	static_assert(sizeof(boost::posix_time::ptime) == sizeof(reference),
	              "We can't save boost ptime as integer");

	date_stream.imbue(_input_locale_facet);
	date_stream >> c.as_time;

	if (c.as_time.is_not_a_date_time()) {
		return false;
	}

	// restrict year if not specified in parameters to match our valid range
	if (c.as_time.date().year() == 1400 and not _is_year_specified) {
		const auto d = c.as_time.date();
		const auto t = c.as_time.time_of_day();
		cal normalized{{boost::posix_time::ptime(
		    boost::gregorian::date(1970, d.month(), d.day()),
		    boost::posix_time::time_duration(t.hours(), t.minutes(), t.seconds(),
		                                     t.fractional_seconds()))}};
		c.as_time = normalized.as_time;
	}

	value = c.as_time;

	// restrict supported value range to [1970..current_year+100[ in order to circonvent some boost
	// bugs...
	return c.as_time.date().year() >= 1970 and c.as_time.date().year() < (current_year + 100);
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_us::convert_to_string
 *****************************************************************************/

int pvcop::types::formatter_datetime_us::convert_to_string(char* str,
                                                           size_t str_len,
                                                           const type& value) const
{
	std::stringstream date_stream;
	date_stream.imbue(_output_locale_facet);

	// Report a bad value rather than a date boost cannot make sense of. Zero is not
	// the only such value: a ptime holds a microsecond count, and any count below a
	// full day makes boost compute a day number of 0, out of which it derives year
	// 60823 and throws bad_year. Since the bounds are boost's own and it signals the
	// failure by throwing, catching is more honest than restating them here.
	// Both cases happen when pvcop memory is not written during import.
	auto report_bad_value = [&str]() {
		std::string bad_value("bad value");
		std::copy(bad_value.begin(), bad_value.end(), str);
		str[bad_value.size()] = '\0';
		return int(bad_value.size());
	};

	if (cal(value).as_value == 0) {
		return report_bad_value();
	}

	cal c(value);

	try {
		date_stream << c.as_time;
	} catch (const std::out_of_range&) {
		// boost::gregorian::bad_year and its siblings all derive from std::out_of_range
		return report_bad_value();
	}

	const std::string& date = date_stream.str();

	if (date.size() > str_len) {
		return -1;
	}

	std::copy(date.begin(), date.end(), str);
	str[date.size()] = '\0';

	return date.size();
}
