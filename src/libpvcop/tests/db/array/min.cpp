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
#include <pvcop/db/algo.h>

#include <common/squey_assert.h>

#include <pvlogger.h>

#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 100000000;
#else
static constexpr size_t SIZE = 100000;
#endif

using namespace pvcop;

int main()
{
	using type_t = uint32_t;
	std::string type = "number_uint32";

	db::array array(type, SIZE);
	core::array<uint32_t>& native_array = array.to_core_array<type_t>();
	std::iota(native_array.begin(), native_array.end(), 0);

	auto rd = std::random_device {};
	auto rng = std::default_random_engine { rd() };
	std::shuffle(native_array.begin(), native_array.begin() + 1000, rng);

	auto start = std::chrono::system_clock::now();

	db::array min1 = array.min();

	auto t2 = std::chrono::system_clock::now();

	db::array min2 = db::algo::min(array);

	auto end = std::chrono::system_clock::now();

#ifdef PVCOP_BENCH
	std::chrono::duration<double> diff = end - start;
	std::cout << diff.count();
#else
	std::chrono::duration<double> diff1 = t2 - start;
	pvlogger::info() << "min array in sequential without sel (sum=" << min1.at(0) << ") done in "
	                 << diff1.count() << " sec." << std::endl;

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff2 = end - t2;

	pvlogger::info() << "min array in parallel without sel   (sum=" << min2.at(0) << ") done in "
	                 << diff2.count() << " sec." << std::endl;

	PV_ASSERT_VALID((min1 == min2 && min1.at(0) == "0"));
#endif
}
