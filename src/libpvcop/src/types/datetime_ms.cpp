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

#include <pvcop/types/datetime_ms.h>

#include <unicode/gregocal.h>

#include <cinttypes>
#ifdef _WIN32
#include <stdlib.h> // _putenv_s for Windows
#endif

/*****************************************************************************
 * datetime_class
 *****************************************************************************/

static pvcop::types::formatter_datetime_ms::EClass datetime_class(const char* parameters)
{
	if (strncmp(parameters, "epoch", 5) == 0) {
		if (parameters[5] == 'S') {
			return pvcop::types::formatter_datetime_ms::EClass::EPOCH;
		} else {
			assert(strncmp(parameters + 5, ".S", 2) == 0);
			return pvcop::types::formatter_datetime_ms::EClass::EPOCH_WITH_DOT;
		}
	} else {
		return pvcop::types::formatter_datetime_ms::EClass::CALENDAR;
	}
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_ms::formatter_datetime_ms
 *****************************************************************************/

// See http://bugs.icu-project.org/trac/ticket/11778
pvcop::types::formatter_datetime_ms::formatter_datetime_ms(const char* parameters)
	: formatter<uint64_t>(parameters), _datetime_class(datetime_class(parameters))
{
	set_parameters(parameters);

	setlocale(LC_ALL, "C");
#ifdef _WIN32
	_putenv_s("LANG", "C");
	_putenv_s("TZ", "GMT");
#else
	setenv("LANG", "C", 1);
	setenv("TZ", "GMT", 1);
#endif
	tzset();
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_ms::set_parameters
 *****************************************************************************/

void pvcop::types::formatter_datetime_ms::set_parameters(const char* parameters)
{
	formatter<uint64_t>::set_parameters(parameters);
	_pattern = icu::UnicodeString(parameters);
	UErrorCode err = U_ZERO_ERROR;
	_dfmt.reset(new icu::SimpleDateFormat(_pattern, err));
	_dfmt->applyPattern(_pattern);
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_ms::convert_from_string
 *****************************************************************************/

bool pvcop::types::formatter_datetime_ms::convert_from_string(const char* str,
                                                              reference value,
                                                              bool* /*pass_autodetect*/) const
{
	static const size_t current_year = []() {
		UErrorCode err = U_ZERO_ERROR;
		UDate now = icu::Calendar::getNow();
		std::unique_ptr<icu::Calendar> c(
		    icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));
		c->setTime(now, err);
		return c->get(UCAL_YEAR, err);
	}();

	static const size_t min_date = []() {
		UErrorCode err = U_ZERO_ERROR;
		std::unique_ptr<icu::Calendar> c(
		    icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));
		c->set(1970, 1, 1, 0, 0, 0);
		return static_cast<uint64_t>(c->getTime(err));
	}();

	static const size_t max_date = []() {
		UErrorCode err = U_ZERO_ERROR;
		std::unique_ptr<icu::Calendar> c(
		    icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));
		c->set(current_year + 100, 1, 1, 0, 0, 0);
		return static_cast<uint64_t>(c->getTime(err));
	}();

	static const size_t min_date_sec = min_date / 1000;
	static const size_t max_date_sec = max_date / 1000;

	switch (_datetime_class) {
	case EClass::EPOCH: {
		if (sscanf(str, "%" SCNu64, &value) != 1 or (value < min_date or value > max_date)) {
			return false;
		}

		return true;
	}
	case EClass::EPOCH_WITH_DOT: {
		uint64_t sec;
		uint64_t msec;

		if (sscanf(str, "%" PRIu64 "%*[.:,]%3" PRIu64, &sec, &msec) != 2 or
		    (sec < min_date_sec or sec > max_date_sec)) {
			return false;
		}

		value = static_cast<uint64_t>(sec * 1000 + msec);

		return true;
	}
	case EClass::CALENDAR: {
		UErrorCode err = U_ZERO_ERROR;

		icu::Calendar* calendar(icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));
		if (not U_SUCCESS(err)) {
			delete calendar;
			return false;
		}

		icu::ParsePosition p(0);
		if (not U_SUCCESS(err)) {
			delete calendar;
			return false;
		}

		/**
		 * Force time to 0 before parsing, otherwise fields that
		 * are not present in the pattern are not initialized !
		 */
		calendar->setTime((UDate)0, err);
		if (not U_SUCCESS(err)) {
			delete calendar;
			return false;
		}
		_dfmt->parse(icu::UnicodeString(str), *calendar, p);
		if (p.getIndex() == 0) {
			delete calendar;
			return false;
		}

		value = static_cast<uint64_t>(calendar->getTime(err));

		bool res = U_SUCCESS(err);

		const size_t year = calendar->get(UCAL_YEAR, err);
		delete calendar;

		return res and (year >= 1970 and year < current_year + 100);
	}
	}

	assert(false);
	return false;
}

/*****************************************************************************
 * pvcop::types::formatter_datetime_ms::convert_to_string
 *****************************************************************************/

int pvcop::types::formatter_datetime_ms::convert_to_string(char* str,
                                                           size_t str_len,
                                                           const type& value) const
{
	switch (_datetime_class) {
	case EClass::EPOCH: {
		return snprintf(str, str_len, "%" PRIu64, value);
	}
	case EClass::EPOCH_WITH_DOT: {
		double v = (double)value / 1000;
		return snprintf(str, str_len, "%.3f", v);
	}
	case EClass::CALENDAR: {
		UErrorCode err = U_ZERO_ERROR;

		thread_local static std::unique_ptr<icu::Calendar> calendar(
		    icu::Calendar::createInstance(icu::TimeZone::getUnknown(), err));
		if (not U_SUCCESS(err)) {
			return -1;
		}

		calendar->setTime(static_cast<UDate>(value), err);
		if (not U_SUCCESS(err)) {
			return -2;
		}

		if (not U_SUCCESS(err)) {
			return -3;
		}

		icu::UnicodeString res;
		icu::FieldPosition fp(0);
		_dfmt->format(*calendar, res, fp);
		if (not U_SUCCESS(err)) {
			return -4;
		}

		std::string s;
		res.toUTF8String(s);

		if (s.size() > str_len) {
			return -5;
		}

		std::copy(s.begin(), s.end(), str);
		str[s.size()] = '\0';

		return s.size();
	}
	}

	assert(false);
	return 0;
}
