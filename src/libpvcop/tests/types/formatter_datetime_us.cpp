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
#include <pvcop/types/datetime_us.h>

#include "formatter_check.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

using cal = pvcop::types::formatter_datetime_us::cal;

static const size_t current_year = boost::gregorian::day_clock::universal_day().year();

static constexpr const size_t min_year = 1970;
static const size_t max_year = current_year + 99;

/*
 * Generates valid boost::posix_time::ptime from a random value
 */
static boost::posix_time::ptime generate_valid_timestamp(uint64_t random_value)
{
	// mask values to improve the repartition range of dates
	uint16_t year = min_year + ((random_value >> 48) % (max_year - min_year));
	uint8_t month = 1 + (random_value >> 44 & 0xF) % 12;
	uint8_t day = 1 + (random_value >> 40 & 0xF) % 28;
	uint8_t hour = (random_value >> 32 & 0xFF) % 24;
	uint8_t minute = (random_value >> 24 & 0xFF) % 60;
	uint8_t sec = (random_value >> 16 & 0xFF) % 60;
	uint32_t usec = ((random_value & 0xFFFF) % 1000000) + 1;

	cal p{{boost::gregorian::date{year, month, day},
	       boost::posix_time::time_duration{hour, minute, sec, usec}}};

	return p.as_time;
}

/**
 * This check is necessary to allow the reinterpret_cast
 * optimisation in formatter_datetime_us in order to be
 * able to sort values.
 */
static void check_ptime_linearity()
{
	const std::vector<std::tuple<uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t>>
	    fields{std::make_tuple(min_year, 1, 1, 0, 0, 0, 0),
	           std::make_tuple(max_year, 1, 1, 0, 0, 0, 0),
	           std::make_tuple(max_year, 12, 1, 0, 0, 0, 0),
	           std::make_tuple(max_year, 12, 31, 0, 0, 0, 0),
	           std::make_tuple(max_year, 12, 31, 23, 0, 0, 0),
	           std::make_tuple(max_year, 12, 31, 23, 59, 0, 0),
	           std::make_tuple(max_year, 12, 31, 23, 59, 59, 0),
	           std::make_tuple(max_year, 12, 31, 23, 59, 59, 999999)};

	for (size_t i = 0; i < fields.size() - 1; i++) {
		const auto& d1 = fields[i];
		const auto& d2 = fields[i + 1];

		cal p1{{boost::gregorian::date{std::get<0>(d1), std::get<1>(d1), std::get<2>(d1)},
		        boost::posix_time::time_duration{std::get<3>(d1), std::get<4>(d1), std::get<5>(d1),
		                                         std::get<6>(d1)}}};

		cal p2{{boost::gregorian::date{std::get<0>(d2), std::get<1>(d2), std::get<2>(d2)},
		        boost::posix_time::time_duration{std::get<3>(d2), std::get<4>(d2), std::get<5>(d2),
		                                         std::get<6>(d2)}}};

		// Check that ptime inner values are sorted
		PV_ASSERT_VALID(p1.as_value < p2.as_value);
	}

	boost::posix_time::ptime last_pt =
	    boost::posix_time::ptime(boost::gregorian::date(min_year, 1, 1));
	for (size_t year = min_year + 1; year < max_year; year++) {
		boost::posix_time::ptime pt(boost::gregorian::date(year, 1, 1));
		PV_ASSERT_VALID(cal{{last_pt}}.as_value < cal{{pt}}.as_value);
		last_pt = pt;
	}

	last_pt = boost::posix_time::ptime(boost::gregorian::date(min_year, 1, 1));
	for (size_t month = 2; month <= 12; month++) {
		boost::posix_time::ptime pt(boost::gregorian::date(min_year, month, 1));
		PV_ASSERT_VALID(cal{{last_pt}}.as_value < cal{{pt}}.as_value);
		last_pt = pt;
	}

	last_pt = boost::posix_time::ptime(boost::gregorian::date(min_year, 1, 1));
	for (size_t day = 2; day <= 28; day++) {
		boost::posix_time::ptime pt(boost::gregorian::date(min_year, 1, day));
		PV_ASSERT_VALID(cal{{last_pt}}.as_value < cal{{pt}}.as_value);
		last_pt = pt;
	}
}

static void check_null_ptime()
{
	static constexpr const size_t SIZE = 10;

	pvcop::db::array array("number_uint64", SIZE);
	auto& array_native = array.to_core_array<uint64_t>();
	std::fill(array_native.begin(), array_native.end(), 0);

	auto formatter_time = std::shared_ptr<pvcop::types::formatter_interface>(
	    pvcop::types::factory::create("datetime_us", "%Y.%m.%d %H:%M:%S.%f"));
	array.set_formatter(formatter_time);

	for (size_t i = 0; i < SIZE; i++) {
		PV_VALID(array.at(i), std::string("bad value"));
	}
}

int main()
{
	// Bench formatter_datetime_us
	std::cout << "formatter_datetime_us:" << std::endl;
	pvcop::types::formatter_datetime_us dtf_us("%Y.%m.%d %H:%M:%S.%f");

	// boost supported year range is 1400..10000 but we only support [1970..current_year+100[ for
	// parsing
	bench_formatter<boost::posix_time::ptime, uint64_t>(
	    dtf_us, std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max(),
	    generate_valid_timestamp);

#ifndef PVCOP_BENCH
	check_ptime_linearity();
	check_null_ptime();
#endif
}
