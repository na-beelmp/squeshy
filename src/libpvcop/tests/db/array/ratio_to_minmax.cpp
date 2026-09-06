//
// MIT License
//
// © Squey, 2026
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

#include <pvcop/db/array.h>

#include "common/squey_assert.h"

#include <pvlogger.h>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace pvcop;
using ptime = boost::posix_time::ptime;

/**
 * ratio_to_minmax turns a ratio of a time range back into the timestamps it names, and it
 * counts that range in microseconds. Half an hour is already more of those than 32 bits
 * hold, so a span of any realistic length overflowed anything narrower than 64: the upper
 * bound came back below the lower one, and every range query against it answered nothing.
 * That draws as a blank plot rather than a wrong one, which is why it went unnoticed
 * wherever long happens to be wide enough.
 *
 * Checked from well under the 32-bit microsecond ceiling to well over it.
 */
static void check_span(ptime first, boost::posix_time::time_duration span, const char* what)
{
	const ptime last = first + span;

	db::array times("datetime_us", 2);
	auto& native = times.to_core_array<ptime>();
	native[0] = first;
	native[1] = last;

	const db::array minmax = times.minmax();

	// The whole range asked for as a ratio has to name the range it is a ratio of.
	const db::array full = times.ratio_to_minmax(0., 1., minmax);
	const auto& full_native = full.to_core_array<ptime>();
	PV_ASSERT_VALID(full_native[0] == first, "span", std::string(what), "bound",
	                std::string("lower"));
	PV_ASSERT_VALID(full_native[1] == last, "span", std::string(what), "bound",
	                std::string("upper"));

	// And a ratio inside it has to stay inside it, in order.
	const db::array half = times.ratio_to_minmax(0.25, 0.75, minmax);
	const auto& half_native = half.to_core_array<ptime>();
	PV_ASSERT_VALID(half_native[0] > first, "span", std::string(what));
	PV_ASSERT_VALID(half_native[1] < last, "span", std::string(what));
	PV_ASSERT_VALID(half_native[0] < half_native[1], "span", std::string(what));
}

int main()
{
	const ptime origin(boost::gregorian::date(2015, 4, 12),
	                   boost::posix_time::time_duration(8, 36, 48) +
	                       boost::posix_time::microseconds(890000));

	// Under the 32-bit microsecond ceiling, which is about 35 minutes.
	check_span(origin, boost::posix_time::minutes(10), "ten minutes");

	// Just over it.
	check_span(origin, boost::posix_time::hours(1), "one hour");

	// Far over it: the kind of range a series view is normally asked for.
	check_span(origin, boost::posix_time::hours(24 * 453), "fifteen months");

	pvlogger::info() << "ratio_to_minmax holds over a range longer than 32 bits of microseconds"
	                 << std::endl;
}
