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

#include <chrono>
#include <common/squey_assert.h>

#include <pvlogger.h>

using namespace pvcop;

#ifdef PVCOP_BENCH
static constexpr size_t count = 200000000;
#else
static constexpr size_t count = 10000000;
#endif
static constexpr size_t sel_ratio = 2; // Mean one bit is set over 2

/**
 * Check bit_count correctly count the number of setted bits and mesure its performance.
 */
int main()
{
	core::memarray<bool> sel(count);
	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = static_cast<bool>(i % sel_ratio == true);
	}

	auto start = std::chrono::system_clock::now();

	// Perform computation
	size_t sel_count = core::algo::bit_count(sel);

	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> diff = end - start;
#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	pvlogger::info() << "bit count (count=" << sel_count << ") done in " << diff.count() << " sec."
	                 << std::endl;

	pvlogger::fatal() << "sel_count=" << sel_count << " (count/sel_ratio)=" << (count / sel_ratio)
	                  << std::endl;
#endif
	// Check result based on ratio
	PV_ASSERT_VALID((sel_count == (count / sel_ratio)));
}
