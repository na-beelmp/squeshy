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

#include <pvcop/collection.h>
#include <pvcop/db/algo.h>
#include <pvcop/db/types.h>
#include <pvcop/collector.h>
#include <pvcop/collection.h>
#include <types/common/import_pipeline.h>

#include <algorithm>
#include <chrono>

#include <common/squey_assert.h>

#ifdef PVCOP_BENCH
static constexpr size_t ELEM_SIZE = 100000000;
#else
static constexpr size_t ELEM_SIZE = 100000;
#endif

static constexpr size_t DISTINCT_VALUES_RATIO = 1000;
static constexpr size_t DEFAULT_VALUE = 7;
static constexpr size_t DISTINCT_VALUES_COUNT = ELEM_SIZE / DISTINCT_VALUES_RATIO;

int main()
{
	using type1 = double;
	pvcop::db::type_t t1 = "number_double";
	using type2 = double;
	pvcop::db::type_t t2 = "number_double";

	pvcop::db::array array_out_1;
	pvcop::db::array array_out_2;

	pvcop::db::array array_in_1(t1, ELEM_SIZE);
	pvcop::db::array array_in_2(t2, ELEM_SIZE);

	auto& array_in_1_native = array_in_1.to_core_array<type1>();
	auto& array_in_2_native = array_in_2.to_core_array<type2>();

	for (size_t i = 0; i < ELEM_SIZE; i++) {
		array_in_1_native[i] = i % DISTINCT_VALUES_COUNT;
	}
	std::fill(array_in_2_native.begin(), array_in_2_native.end(), DEFAULT_VALUE);

	auto start = std::chrono::system_clock::now();

	pvcop::db::algo::average_by(array_in_1, array_in_2, array_out_1, array_out_2);

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "average_by with " << ELEM_SIZE << " elements took: " << diff.count() << " sec"
	          << std::endl;

	bool sizes_ok =
	    array_out_1.size() == array_out_2.size() && array_out_1.size() == DISTINCT_VALUES_COUNT;
	PV_ASSERT_VALID(sizes_ok);

	auto& array_out_1_native = array_out_1.to_core_array<type1>();

	std::sort(array_out_1_native.begin(), array_out_1_native.end());
	bool group = std::unique(array_out_1_native.begin(), array_out_1_native.end()) ==
	             array_out_1_native.end();
	group &= *array_out_1_native.begin() == 0;
	group &= *std::prev(array_out_1_native.end()) == DISTINCT_VALUES_COUNT - 1;
	// TODO : implement core::array::front() and core::array::back()

	PV_ASSERT_VALID(group);

	const auto& array_out_2_native = array_out_2.to_core_array<type2>();

	bool avg_ok = std::all_of(array_out_2_native.begin(), array_out_2_native.end(),
	                          [&](type2 val) { return val == DEFAULT_VALUE; });
	PV_ASSERT_VALID(avg_ok);
#endif
}
