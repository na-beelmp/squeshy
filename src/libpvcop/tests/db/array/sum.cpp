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

#include <pvcop/core/memarray.h>
#include <pvcop/db/array.h>
#include <pvcop/db/algo.h>

#include <common/squey_assert.h>

#include <pvlogger.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <numeric>

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 200000000;
#else
static constexpr size_t SIZE = 100000;
#endif

using namespace pvcop;

int main()
{
	using type_t = double;
	std::string type = "number_double";

	db::array array(type, SIZE);
	core::array<type_t>& native_array = array.to_core_array<type_t>();
	std::iota(native_array.begin(), native_array.end(), 0.0);

	pvcop::core::memarray<bool> sel(SIZE);
	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = (i % 2);
	}

	auto start = std::chrono::system_clock::now();

	db::array sum_array1 = array.sum(sel);
	double sum1 = sum_array1.to_core_array<double>()[0];

	auto t2 = std::chrono::system_clock::now();

	db::array sum_array2 = db::algo::sum(array, sel);
	double sum2 = sum_array2.to_core_array<double>()[0];

	auto end = std::chrono::system_clock::now();

#ifdef PVCOP_BENCH
	std::chrono::duration<double> diff = end - start;
	std::cout << diff.count();
#else
	std::chrono::duration<double> diff1 = t2 - start;
	pvlogger::info() << "sum " << SIZE << " elements in sequential without sel (sum=" << sum1
	                 << ") done in " << diff1.count() << " sec." << std::endl;

	end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff2 = end - t2;

	pvlogger::info() << "sum " << SIZE << " elements in parallel without sel   (sum=" << sum2
	                 << ") done in " << diff2.count() << " sec." << std::endl;

	pvlogger::info() << "espected sum   = " << std::fixed << std::setprecision(6)
	                 << (double)((SIZE * (SIZE - 1)) / 2) << std::endl;
	pvlogger::info() << "sequential sum = " << std::fixed << std::setprecision(6) << sum1
	                 << std::endl;
	pvlogger::info() << "parallel sum   = " << std::fixed << std::setprecision(6) << sum2
	                 << std::endl;

	PV_VALID(sum1, (double)(SIZE / 2) * (SIZE / 2));
	PV_VALID(sum1, sum2);
#endif
}
