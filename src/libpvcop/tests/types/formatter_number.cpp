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

#include <pvcop/db/array.h>
#include <pvcop/types/factory.h>

#include <common/squey_assert.h>

#include <iostream>
#include <random>
#include <cinttypes> // PRIxxx macro
#include <unordered_map>

#define protected public
#include <pvcop/types/number.h>

using namespace pvcop;

#ifdef PVCOP_BENCH
constexpr size_t SIZE = 100000000;
#else
constexpr size_t SIZE = 1000000;
#endif

static constexpr size_t MAX_STR_LENGTH = 1024;
std::vector<std::string> input_strings;
std::vector<std::string> output_strings;

// clang-format off
static const std::unordered_map<std::string, std::vector<std::vector<std::string>>> OVERFLOWS = {
    {{"int8",  {{ "-129", "128" }}},
     {"int16", {{ "-32769", "32768" }}},
     {"int32", {{ "-2147483649", "2147483648" }}},
     {"int64", {{ "-9223372036854775809", "9223372036854775808" }}},
     {"uint8", {{ "-1", "256" } ,
                { "0x100"     },
                { "0400"      }}},
     {"uint16",{{ "-1", "65536" },
                { "0x10000"     },
                { "0200000"     }}},
     {"uint32",{{ "-1", "4294967296" },
                { "0x100000000"      },
                { "040000000000"     }}},
     {"uint64",{{ "-1", "18446744073709551616" },
                { "0x10000000000000000"        },
                { "02000000000000000000000"    }}},
     {"float", {{ "-3.40282e+39", "3.40282e+39" } }},
     {"double",{{ "-1.79769e+309", "1.79769e+309" } }}}};
// clang-format on

template <typename D>
double test(const char* type, const std::vector<const char*> opts, D& distrib)
{
	using type_t = typename D::result_type;

	pvcop::core::memarray<type_t> core_array(SIZE);

	std::chrono::duration<double> parse_time(0.);
	std::chrono::duration<double> format_time(0.);

	size_t opt_idx = 0;
	for (const char* opt : opts) {
		std::chrono::duration<double> ptime(0.);
		std::chrono::duration<double> ftime(0.);

		// Set proper formatter
		pvcop::types::formatter_number<type_t> formatter(opt);

		// Generate random strings
		std::default_random_engine generator;
#pragma omp parallel for
		for (size_t i = 0; i < SIZE; i++) {
			char buff[MAX_STR_LENGTH];
			snprintf(buff, MAX_STR_LENGTH, opt, distrib(generator));
			input_strings[i] = buff;
		}

		// parse
		{
			auto start = std::chrono::system_clock::now();

			type_t* data = static_cast<type_t*>(core_array.data());
#pragma omp parallel for
			for (size_t i = 0; i < SIZE; i++) {
				formatter.convert_from_string(input_strings[i].c_str(), data[i], nullptr);
			}

			auto end = std::chrono::system_clock::now();
			ptime += end - start;
		}

		// format
		{
			auto start = std::chrono::system_clock::now();

			type_t* data = static_cast<type_t*>(core_array.data());
#pragma omp parallel for
			for (size_t i = 0; i < SIZE; i++) {
				char str[MAX_STR_LENGTH];
				formatter.convert_to_string(str, MAX_STR_LENGTH, data[i]);
			}

			auto end = std::chrono::system_clock::now();
			ftime += end - start;

#pragma omp parallel for
			for (size_t i = 0; i < SIZE; i++) {
				char str[MAX_STR_LENGTH];
				formatter.convert_to_string(str, MAX_STR_LENGTH, data[i]);
				output_strings[i] = str;
			}
		}

// validate
#ifndef PVCOP_BENCH
#pragma omp parallel for
		for (size_t i = 0; i < SIZE; i++) {
			PV_VALID(output_strings[i], input_strings[i]);
		}

		// Check overflow handling
		type_t value;
		auto overflow_strings = OVERFLOWS.at(type)[opt_idx];
		for (const std::string& overflow_string : overflow_strings) {
			bool res = formatter.from_string(overflow_string.c_str(), &value, 0);
			if (res) {
				pvlogger::error() << overflow_string.c_str() << std::endl;
			}
			PV_VALID(res, false);
		}

		opt_idx++;

		pvlogger::info() << "parsing    " << SIZE << " " << type << "(" << opt << ") values took "
		                 << ptime.count() << " sec" << std::endl;
		pvlogger::info() << "formatting " << SIZE << " " << type << "(" << opt << ") values took "
		                 << ftime.count() << " sec" << std::endl;
#else
		(void)opt_idx;
		(void)type;
#endif

		parse_time += ptime;
		format_time += ftime;
	}

	return parse_time.count() + format_time.count();
}

int main()
{
	output_strings.resize(SIZE);
	input_strings.resize(SIZE);

	double total_time = 0.0;

	// test integers
	std::uniform_int_distribution<uint8_t> uint8_dist(std::numeric_limits<uint8_t>::min(),
	                                                  std::numeric_limits<uint8_t>::max());

	total_time = test("uint8", {"%" PRIu8, "0x%" PRIxPTR, "%#o"}, uint8_dist);

	std::uniform_int_distribution<int8_t> int8_dist(std::numeric_limits<int8_t>::min(),
	                                                std::numeric_limits<int8_t>::max());
	total_time += test("int8", {"%" PRId8}, int8_dist);

	std::uniform_int_distribution<uint16_t> uint16_dist(std::numeric_limits<uint16_t>::min(),
	                                                    std::numeric_limits<uint16_t>::max());

	total_time = test("uint16", {"%" PRIu16, "0x%" PRIxPTR, "%#o"}, uint16_dist);

	std::uniform_int_distribution<int16_t> int16_dist(std::numeric_limits<int16_t>::min(),
	                                                  std::numeric_limits<int16_t>::max());
	total_time += test("int16", {"%" PRId16}, int16_dist);

	std::uniform_int_distribution<uint32_t> uint32_dist(std::numeric_limits<uint32_t>::min(),
	                                                    std::numeric_limits<uint32_t>::max());

	total_time = test("uint32", {"%" PRIu32, "0x%" PRIxPTR, "%#o"}, uint32_dist);

	std::uniform_int_distribution<int32_t> int32_dist(std::numeric_limits<int32_t>::min(),
	                                                  std::numeric_limits<int32_t>::max());
	total_time += test("int32", {"%" PRId32}, int32_dist);

	std::uniform_int_distribution<uint64_t> uint64_dist(std::numeric_limits<uint64_t>::min(),
	                                                    std::numeric_limits<uint64_t>::max());

	total_time += test("uint64", {"%" PRIu64, "0x%" PRIxPTR, "%#lo"}, uint64_dist);

	std::uniform_int_distribution<int64_t> int64_dist(std::numeric_limits<int64_t>::min(),
	                                                  std::numeric_limits<int64_t>::max());
	total_time += test("int64", {"%" PRId64}, int64_dist);

	// test reals
	std::uniform_real_distribution<float> float_dist(-3.40282e+4, +3.40282e+4);
	total_time += test("float", {"%.9g"}, float_dist);

	std::uniform_real_distribution<double> double_dist(-1.79769e+307, 1.79769e+307);
	total_time += test("double", {"%.17g"}, double_dist);

	(void) total_time;
#ifdef PVCOP_BENCH
	std::cout << total_time;
#endif
}
