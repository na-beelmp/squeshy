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

#include <pvcop/db/algo.h>
#include <pvcop/db/types.h>
#include <pvcop/core/memarray.h>
#include <pvcop/types/factory.h>
#include <pvcop/db/selection.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <numeric>

#include "common/squey_assert.h"

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 200000000;
#else
static constexpr size_t SIZE = 100000;
#endif

int main()
{
	using type_t = uint32_t;
	std::string type = "number_uint32";

	// Selection after subselect range
	pvcop::core::memarray<bool> out_sel(SIZE);

	pvcop::db::array array_in_1(type, SIZE);

	// Fill with increasing values (0 to SIZE) the db array
	auto& array_in_1_native = array_in_1.to_core_array<type_t>();
	std::iota(array_in_1_native.begin(), array_in_1_native.end(), 0);

	// Select one element over 2
	pvcop::core::memarray<bool> sel(array_in_1.size());
	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = i % 3;
	}
	auto start = std::chrono::system_clock::now();

	pvcop::db::algo::range_select(array_in_1, std::to_string((type_t)(SIZE * 0.4)),
	                              std::to_string((type_t)(SIZE * 0.7)), sel, out_sel);

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "pvcop::db::algo::range_select " << SIZE << " elements took: " << diff.count()
	          << " sec" << std::endl;

	// Check proper content
	for (size_t i = 0; i < out_sel.size(); i++) {
		if (i >= SIZE * 0.4 and i <= SIZE * 0.7 and i % 3) {
			PV_ASSERT_VALID(static_cast<bool>(out_sel[i]));
		} else {
			PV_ASSERT_VALID(not out_sel[i]);
		}
	}

	// Check with ipv4
	auto formatter_ipv4 = std::shared_ptr<pvcop::types::formatter_interface>(
	    pvcop::types::factory::create("ipv4", ""));
	array_in_1.set_formatter(formatter_ipv4);

	std::fill(sel.begin(), sel.end(), true);
	pvcop::db::algo::range_select(array_in_1, "0.0.0.0", "0.0.255.255", sel, out_sel);
	const size_t bit_count = pvcop::core::algo::bit_count(out_sel);

	PV_VALID(bit_count, (size_t)256 * 256);
#endif // PVCOP_BENCH
}
