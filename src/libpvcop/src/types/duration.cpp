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

#include <pvcop/types/duration.h>

#include <cinttypes>

/*****************************************************************************
 * pvcop::types::formatter_duration::convert_from_string
 *****************************************************************************/

bool pvcop::types::formatter_duration::convert_from_string(const char* str,
                                                           reference value,
                                                           bool* /*pass_autodetect*/) const
{
	uint32_t hour = 0;
	uint32_t min = 0;
	uint32_t sec = 0;
	uint32_t msec = 0;

	const bool negative = str[0] == '-';
	if (negative) {
		str++;
	}

	char pattern[5]{};
	size_t pattern_size = 0;
	for (size_t i = 0; str[i] != '\0' and pattern_size < sizeof(pattern) - 1; i++) {
		if (not std::isdigit(str[i])) {
			pattern[pattern_size++] = str[i];
		}
	}

	int res = 0;
	if (pattern_size <= 3) {
		if (strcmp(pattern, ".") == 0) {
			double duration, intpart;
			res = sscanf(str, "%lf", &duration);
			if (res) {
				msec = (uint32_t)(std::modf(duration, &intpart) * 1000);
				sec = (size_t)duration % 60;
				min = ((size_t)duration % 3600) / 60;
				hour = (size_t)duration / 3600;
			}
		} else if (strcmp(pattern, ":") == 0 or strcmp(pattern, ":.") == 0) {
			res = sscanf(str, "%2d:%2d.%d", &min, &sec, &msec);
		} else if (strcmp(pattern, "::") == 0 or strcmp(pattern, "::.") == 0) {
			res = sscanf(str, "%d:%2d:%2d.%d", &hour, &min, &sec, &msec);
		}
	}

	const type duration(hour, min, sec, msec);
	value = negative ? duration.invert_sign() : duration;

	return res >= 1;
}

/*****************************************************************************
 * pvcop::types::formatter_duration::convert_to_string
 *****************************************************************************/

int pvcop::types::formatter_duration::convert_to_string(char* str,
                                                        size_t max_len,
                                                        const type& value) const
{
	// Every component of a negative duration is negative, so format the absolute value
	// and prepend the sign.
	const bool negative = value.is_negative();
	const type absolute_value = negative ? value.invert_sign() : value;

	const int64_t days = absolute_value.hours() / 24;
	const int64_t hours = absolute_value.hours() % 24;
	const int64_t mins = absolute_value.minutes();
	const int64_t secs = absolute_value.seconds();

	size_t str_len = 0;
	if (negative) {
		str_len += snprintf(str, max_len, "-");
	}
	if (days > 0) { // optional number of days
		str_len += snprintf(str + str_len, max_len - str_len, "%" PRId64 ":", days);
	}
	str_len += snprintf(str + str_len, max_len - str_len, "%02" PRId64 ":%02" PRId64 ":%02" PRId64,
	                    hours, mins, secs);
	if (absolute_value.fractional_seconds() != 0) { // optional microseconds
		str_len += snprintf(str + str_len, max_len - str_len, ".%06" PRId64,
		                    absolute_value.fractional_seconds());
	}

	return str_len;
}
