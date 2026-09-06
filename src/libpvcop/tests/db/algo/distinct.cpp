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

#include <algorithm>
#include <chrono>
#include <numeric>

#include <common/squey_assert.h>

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 200000000;
#else
static constexpr size_t SIZE = 128; // ensure that map_reduce bug with 128 elements is fixed
#endif

int main()
{
	using type_t = uint64_t;
	std::string type = "number_uint64";

	pvcop::db::array array_out_1;
	pvcop::db::array array_out_2;

	pvcop::db::array array_in_1(type, SIZE);

	auto& array_in_1_native = array_in_1.to_core_array<type_t>();

	auto mid_iterator = array_in_1_native.begin();
	std::advance(mid_iterator, SIZE / 2);

	std::iota(array_in_1_native.begin(), mid_iterator, 0);
	std::iota(mid_iterator, array_in_1_native.end(), 0);

	pvcop::core::memarray<bool> sel(array_in_1.size());
	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = i % 2;
	}
	auto start = std::chrono::system_clock::now();

	pvcop::db::algo::distinct(array_in_1, array_out_1, array_out_2, sel);

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "pvcop::db::algo::distinct " << SIZE << " elements took: " << diff.count()
	          << " sec" << std::endl;

	auto& array_out_1_native = array_out_1.to_core_array<type_t>();
	auto& array_out_2_native = array_out_2.to_core_array<type_t>();

	// Check size
	PV_ASSERT_VALID(array_out_1.size() == array_out_2.size() && array_out_1.size() == SIZE / 4);

	// Check proper content
	for (size_t i = 0; i < array_out_1.size(); i++) {
		PV_ASSERT_VALID(array_out_1_native[i] == i * 2 + 1);
		PV_ASSERT_VALID(array_out_2_native[i] == 2);
	}

#endif // PVCOP_BENCH
}
