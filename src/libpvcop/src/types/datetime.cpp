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

#include <pvcop/types/datetime.h>

#include <cstring>
#include <ctime>
#include <clocale>
#include <cstdlib>
#ifdef _WIN32
#include <stdlib.h> // _putenv_s for Windows
#include <chrono>
#include <iomanip>
#endif
#include <cinttypes>

#include <unicode/calendar.h>

#include <pvlogger.h>

/**
Bench on 100 000 000 random timestamps
|operation             | sequential | parallel (12 cores) | comments
-----------------------|------------|---------------------|---------------------------------------------
|gmtime_r              | 8 sec      | 24 sec              | glibc impl uses lock that are killing
perfs
|_gmtime_r             | 41 sec     | 6 sec               | ICU Calendar converted to glibc tm
struct
|strfmtime+gmtime_r    | 33 sec     | 35 sec              | lock overhead reduced when doing heavy
stuff
|strfmtime+_gmtime_r   | 70 sec     | 10 sec              | ICU calendar + libc formatting = big win
|ICU (SimpleDateFormat)| 733 sec    | 291 sec             | ICU and UTF16 internal encoding is
sloooooow
*/
pvcop::types::formatter_datetime::formatter_datetime(const char* parameters)
    : formatter<uint64_t>(parameters)
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

	set_parameters(parameters);
}

bool pvcop::types::formatter_datetime::convert_from_string(const char* str,
                                                           reference value,
                                                           bool* /*pass_autodetect*/) const
{
	if (strncmp(parameters(), "%s", 2) == 0) {
		uint64_t v;
		if (sscanf(str, "%" SCNu64, &v) != 1 or (v > std::numeric_limits<int32_t>::max())) {
			return false;
		}

		value = static_cast<uint64_t>(v);
		return true;
	} else {
		thread_local tm local_tm;
		memset(&local_tm, 0, (size_t)sizeof(struct tm));

#ifdef _WIN32
		bool res = true;
		std::istringstream stream(str);
		stream.imbue(std::locale(setlocale(LC_ALL, nullptr)));
		stream >> std::get_time(&local_tm, parameters());
		if (stream.fail()) {
			return false;
		}
#else
		char* ptr = strptime(str, parameters(), &local_tm);
		bool res = ptr != nullptr;
#endif

		// restrict year if not specified in parameters to match our valid range
		if (local_tm.tm_year == 0 and not _is_year_specified) {
			local_tm.tm_year = 70;
		}

		value = static_cast<uint64_t>(pvcop::types::formatter_datetime::mktime(&local_tm));

		return res;
	}
}

int pvcop::types::formatter_datetime::convert_to_string(char* str,
                                                        size_t str_len,
                                                        const type& value) const
{
	thread_local tm local_tm;

#ifdef _WIN32 // std::strftime does not support "%s" specifier under Windows
	if (strncmp(parameters(), "%s", 2) == 0) {
		return snprintf(str, str_len, "%" PRIu64, value);
	}
#endif
	size_t len = std::strftime(str, str_len, parameters(),
	                           pvcop::types::formatter_datetime::gmtime_r(&value, &local_tm));

	return len;
}

/**
 * Drop-in replacement for glibc mktime
 *
 * Using ICU to convert the tm struct to unix timestamp
 * is way more scalable since it doesn't uses an internal lock.
 *
 * @param tm broken-down time structure
 *
 * @return the unix timestamp
 */
time_t pvcop::types::formatter_datetime::mktime(struct tm* tm)
{
	if (!tm) {
		return 0;
	}

	UErrorCode err = U_ZERO_ERROR;
	thread_local static std::unique_ptr<icu::Calendar> calendar(
	    icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));

	if (U_SUCCESS(err) == false) {
		return 0;
	}

	// Normalise value to avoid overflow
	tm->tm_year = std::max(tm->tm_year, 70);
	tm->tm_mday = std::max(tm->tm_mday, 1);

	/**
	 * Initialize ICU calendar with all the fields of the libc tm struct
	 */
	calendar->set(tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

#ifdef _WIN32
	thread_local std::tm local_tm = {};
	time_t local_time = std::mktime(&local_tm);
	std::tm* gmt_tm = std::gmtime(&local_time);
	time_t gmt_time = std::mktime(gmt_tm);
	int32_t gmt_off = static_cast<int32_t>(local_time - gmt_time);
#else
	int32_t gmt_off = tm->tm_gmtoff;
#endif

	calendar->set(UCAL_ZONE_OFFSET, gmt_off * 1000);

	time_t time = calendar->getTime(err) / 1000;

	return time;
}

/**
 * Drop-in replacement for glibc gmtime_r
 *
 * Using ICU to convert the unix timestamp to the tm struct
 * is way more scalable since it doesn't uses an internal lock.
 *
 * @param timep time_t unix timestamp
 * @param result broken-down time structure
 *
 * @return the broken-down time structure
 */
tm* pvcop::types::formatter_datetime::gmtime_r(const uint64_t* timep, tm* result)
{
	if (!timep || !result) {
		return nullptr;
	}

	UErrorCode err = U_ZERO_ERROR;
	thread_local static std::unique_ptr<icu::Calendar> calendar(
	    icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));

	if (U_SUCCESS(err) == false) {
		return nullptr;
	}

	const UDate datetime = *timep * 1000;
	calendar->setTime(datetime, err);

	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_sec = calendar->get(UCAL_SECOND, err);
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_min = calendar->get(UCAL_MINUTE, err);
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_hour = calendar->get(UCAL_HOUR_OF_DAY, err);
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_mday = calendar->get(UCAL_DAY_OF_MONTH, err);
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_mon = calendar->get(UCAL_MONTH, err);
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_year = calendar->get(UCAL_YEAR, err) - 1900;
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_wday = calendar->get(UCAL_DAY_OF_WEEK, err) - 1;
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_yday = calendar->get(UCAL_DAY_OF_YEAR, err) - 1;
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

	result->tm_isdst = calendar->inDaylightTime(err);
	if (!U_SUCCESS(err)) {
		return nullptr;
	}

#ifndef _WIN32
	// glibc additional fields !
	result->tm_gmtoff =
	    (calendar->get(UCAL_ZONE_OFFSET, err) + calendar->get(UCAL_DST_OFFSET, err)) / 1000;
	result->tm_zone = const_cast<char*>("GMT");
#endif

	return result;
}
