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

#include <pvcop/core/algo/selection.h>
#include <pvcop/core/memarray.h>
#include <pvcop/core/selected_array.h>

#include "common/squey_assert.h"

#include <chrono>
#include <numeric>

int test_nth_set_bit()
{
	using value_type = pvcop::core::__impl::bit_manip::value_type;
	pvcop::core::memarray<bool> sel(200);

	PV_VALID(sel.size(), (size_t)200);

	std::fill(sel.begin(), sel.end(), 1);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)200);

	std::fill(sel.begin(), sel.end(), 0);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);

	sel[32] = 1;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 1, 0, sel.size()), (size_t)32);

	sel[33] = 1;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 2, 0, sel.size()), (size_t)33);

	sel[63] = 1;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 3, 0, sel.size()), (size_t)63);

	sel[64] = 1;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 3, 0, sel.size()), (size_t)63);
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 4, 0, sel.size()), (size_t)64);

	std::fill(sel.begin(), sel.end(), 0);

	// FIXME : We should use INVALID_INDEX but selection typedef and selection namespace clash
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 1, 0, sel.size()), (size_t)(-1));

	sel[64] = 1;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 1, 0, sel.size()), (size_t)64);

	sel[192] = 1;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 1, 0, sel.size()), (size_t)64);
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 2, 0, sel.size()), (size_t)192);

	sel[199] = 1;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 3, 0, sel.size()), (size_t)199);

	sel.data()[3] |= (value_type)1 << 10;
	PV_VALID(pvcop::core::algo::find_nth_set_bit(sel, 4, 0, sel.size()), (size_t)(-1));

	return 0;
}

int test_nth_last_set_bit()
{
	pvcop::core::memarray<bool> sel(200);

	PV_VALID(sel.size(), (size_t)200);

	std::fill(sel.begin(), sel.end(), 1);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)200);

	std::fill(sel.begin(), sel.end(), 0);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);

	sel[64] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 1, 0, sel.size()), (size_t)64);

	std::fill(sel.begin(), sel.end(), 0);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);

	sel[63] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 1, 0, sel.size()), (size_t)63);

	sel[33] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 2, 0, sel.size()), (size_t)33);

	sel[0] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 2, 0, sel.size()), (size_t)33);
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 3, 0, sel.size()), (size_t)0);

	sel[0] = 0;
	sel[1] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 2, 0, sel.size()), (size_t)33);
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 3, 0, sel.size()), (size_t)1);

	std::fill(sel.begin(), sel.end(), 0);

	// FIXME : We should use INVALID_INDEX but selection typedef and selection namespace clash
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 1, 0, sel.size()), (size_t)(-1));

	sel[64] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 1, 0, sel.size()), (size_t)64);

	sel[192] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 1, 0, sel.size()), (size_t)192);
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 2, 0, sel.size()), (size_t)64);

	sel[0] = 1;
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 3, 0, sel.size()), (size_t)0);
	PV_VALID(pvcop::core::algo::find_nth_last_set_bit(sel, 3, 1, sel.size()), (size_t)(-1));

	return 0;
}

int main()
{
	test_nth_set_bit();
	test_nth_last_set_bit();

	constexpr size_t SIZE = 1000000;
	pvcop::core::memarray<size_t> arr(SIZE);
	pvcop::core::memarray<bool> sel(SIZE);
	bool mask = false;
	std::generate_n(sel.begin(), SIZE, [&]() {
		mask = not mask;
		return mask;
	});
	std::iota(arr.begin(), arr.end(), 0);
	auto sel_a = pvcop::core::make_selected_array(arr, sel);

	size_t res = 0;
	auto it_end = sel_a.cend();

	auto start = std::chrono::steady_clock::now();
#pragma omp parallel for reduction(+ : res) schedule(guided)
	for (auto it = sel_a.cbegin(); it < it_end; ++it) {
		res += *it;
	}
	auto end = std::chrono::steady_clock::now();

	PV_VALID(res, SIZE * (SIZE - 2) / 4);
	std::cout << std::chrono::duration<double>(end - start).count() << std::endl;
	return 0;
}
