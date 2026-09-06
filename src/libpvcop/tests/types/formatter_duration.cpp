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

#include <pvcop/pvcop.h>
#include <pvcop/types/duration.h>

#include "formatter_check.h"

using duration_t = boost::posix_time::time_duration;

static duration_t generate_valid_durations(uint64_t random_value)
{
	uint32_t hour = (random_value % 24);
	uint8_t minute = (random_value >> 24 & 0xFF) % 60;
	uint8_t sec = (random_value >> 16 & 0xFF) % 60;
	uint16_t msec = (random_value & 0xFFFF) % 1000;

	return duration_t(hour, minute, sec, msec);
}

static duration_t generate_valid_negative_durations(uint64_t random_value)
{
	return generate_valid_durations(random_value).invert_sign();
}

static std::string to_string(const pvcop::types::formatter_duration& formatter,
                             const duration_t& duration)
{
	char str[64]{};
	formatter.to_string(str, sizeof(str), &duration, 0);

	return std::string(str);
}

/**
 * Check the string representation of durations, negative ones included.
 */
static void check_durations_to_string(const pvcop::types::formatter_duration& formatter)
{
	const std::vector<std::pair<duration_t, std::string>> durations = {
	    {duration_t(0, 0, 0, 0), "00:00:00"},
	    {duration_t(1, 2, 3, 0), "01:02:03"},
	    {duration_t(0, 0, 1, 500000), "00:00:01.500000"},
	    {duration_t(25, 1, 1, 0), "1:01:01:01"},
	    {duration_t(1, 2, 3, 0).invert_sign(), "-01:02:03"},
	    {duration_t(0, 0, 1, 500000).invert_sign(), "-00:00:01.500000"},
	    {duration_t(25, 1, 1, 0).invert_sign(), "-1:01:01:01"},
	};

	for (const auto& [duration, expected_string] : durations) {
		PV_VALID(to_string(formatter, duration), expected_string);
	}
}

int main()
{
	// Bench formatter_duration
	std::cout << "formatter_duration:" << std::endl;
	pvcop::types::formatter_duration dur("");

	check_durations_to_string(dur);

	bench_formatter<boost::posix_time::time_duration, uint64_t>(
	    dur, std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max(),
	    generate_valid_durations);

	// Negative durations must survive a to_string/from_string round trip as well
	bench_formatter<boost::posix_time::time_duration, uint64_t>(
	    dur, std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max(),
	    generate_valid_negative_durations);
}
