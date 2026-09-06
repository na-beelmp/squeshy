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
#include <pvcop/core/memarray.h>

#include <common/squey_assert.h>

#include <pvlogger.h>

#include <algorithm>
#include <chrono>
#include <numeric>

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

	pvcop::core::memarray<bool> sel(SIZE);
	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = i % 2 == false;
	}

	auto start = std::chrono::system_clock::now();

	db::array avg_array1 = array.average(sel);
	double avg1 = avg_array1.to_core_array<double>()[0];

	auto t2 = std::chrono::system_clock::now();

	db::array avg_array2 = db::algo::average(array, sel);
	double avg2 = avg_array2.to_core_array<double>()[0];

	auto end = std::chrono::system_clock::now();

#ifdef PVCOP_BENCH
	std::chrono::duration<double> diff = end - start;
	std::cout << diff.count();
#else
	std::chrono::duration<double> diff1 = t2 - start;
	pvlogger::info() << "avg array in sequential without sel (avg=" << avg1 << ") done in "
	                 << diff1.count() << " sec." << std::endl;

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff2 = end - t2;

	pvlogger::info() << "avg array in parallel without sel   (avg=" << avg2 << ") done in "
	                 << diff2.count() << " sec." << std::endl;

	PV_ASSERT_VALID((avg1 == avg2));
	PV_VALID(avg1, ((double)SIZE / 2 - 1));
#endif
}
