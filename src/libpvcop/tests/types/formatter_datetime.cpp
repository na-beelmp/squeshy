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
#include <pvcop/types/datetime.h>

#include "formatter_check.h"

static constexpr const size_t min_year = 1970;
static const size_t max_year = 2037;

/*
 * Generates valid tm values from a random value
 */
static uint64_t generate_valid_timestamp(uint64_t random_value)
{
	tm date = {};

	date.tm_year = 1900 - min_year + ((random_value) % (max_year - min_year));
	date.tm_mon = 1 + (random_value) % 12;
	date.tm_mday = 1 + (random_value) % 28;
	date.tm_hour = (random_value) % 24;
	date.tm_min = (random_value) % 60;
	date.tm_sec = (random_value) % 60;

	return pvcop::types::formatter_datetime::mktime(&date);
}

int main()
{
	// Bench formatter_datetime
	std::cout << "formatter_datetime:" << std::endl;
	pvcop::types::formatter_datetime dtf("%Y.%m.%d 'at' %H:%M:%S %Z");

	bench_formatter<uint64_t, uint64_t>(dtf, std::numeric_limits<uint64_t>::min(),
	                                    std::numeric_limits<uint64_t>::max(),
	                                    generate_valid_timestamp);
}
