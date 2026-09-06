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
#include <pvcop/types/datetime_ms.h>

#include <unicode/gregocal.h>

#include "formatter_check.h"

static const size_t current_year = []() {
	UErrorCode err = U_ZERO_ERROR;
	UDate now = icu::Calendar::getNow();
	std::unique_ptr<icu::Calendar> c(
	    icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));
	c->setTime(now, err);

	return c->get(UCAL_YEAR, err);
}();

static constexpr const size_t min_year = 1970;
static const size_t max_year = current_year + 99;

/*
 * Generates valid tm values from a random value
 */
static uint64_t generate_valid_timestamp(uint64_t random_value)
{
	// mask values to improve the repartition range of dates
	uint16_t year = min_year + ((random_value >> 48) % (max_year - min_year));
	uint8_t month = 1 + (random_value >> 44 & 0xF) % 12;
	uint8_t day = 1 + (random_value >> 40 & 0xF) % 28;
	uint8_t hour = (random_value >> 32 & 0xFF) % 24;
	uint8_t minute = (random_value >> 24 & 0xFF) % 60;
	uint8_t sec = (random_value >> 16 & 0xFF) % 60;
	uint16_t msec = (random_value & 0xFFFF) % 1000;

	UErrorCode err = U_ZERO_ERROR;
	thread_local static std::unique_ptr<icu::Calendar> calendar(icu::Calendar::createInstance(err));
	calendar->set(year, month, day, hour, minute, sec);
	calendar->set(UCAL_MILLISECOND, msec);

	return static_cast<uint64_t>(calendar->getTime(err));
}

int main()
{
	// Bench formatter_datetime_ms
	std::cout << std::endl << "formatter_datetime_ms:" << std::endl;
	pvcop::types::formatter_datetime_ms dtf_ms("yyyy.M.d 'at' a HH:mm:ss,SSS v");
	// ICU have overflow issue for greater values
	bench_formatter<uint64_t, uint64_t>(dtf_ms, std::numeric_limits<uint64_t>::min(),
	                                    std::numeric_limits<uint64_t>::max(),
	                                    generate_valid_timestamp);
}
