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
#include <pvcop/core/memarray.h>

#include <common/squey_assert.h>

#include <random>

#ifdef PVCOP_BENCH
static constexpr size_t DISTINCT_VALUES = 20000;
#else
static constexpr size_t DISTINCT_VALUES = 1000;
#endif

/************************************************************
 * worst case/best case with a complexe selection are tested.
 * Here is the structure of the involved arrays :
 *
 * [ c1| c2|c3|  sel |
 * |---|---|--|------|
 * | 1 | 0 |1 |false |
 * | 2 | 0 |1 |false |
 * | 2 | 1 |1 |true  |
 * | 3 | 0 |1 |false |
 * | 3 | 1 |1 |true  |
 * | 3 | 2 |1 |true  |
 * | 4 | 0 |1 |false |
 * | 4 | 1 |1 |true  |
 * | 4 | 2 |1 |true  |
 * | 4 | 3 |1 |true  |
 * |...|...|..|...   |
 *
 **************************************************************/

int main()
{
	pvcop::db::array array_out_1;
	pvcop::db::array array_out_2;
	pvcop::db::array array_out_3;
	pvcop::db::array array_out_4;

	size_t size = (DISTINCT_VALUES * (DISTINCT_VALUES + 1)) / 2;

	pvcop::db::array array_in_1(pvcop::db::type_index, size);
	pvcop::db::array array_in_2(pvcop::db::type_index, size);
	pvcop::db::array array_in_3(pvcop::db::type_index, size);
	std::vector<bool> tmp_sel(size);
	pvcop::core::memarray<bool> out_sel(size);

	auto& array_in_1_native = array_in_1.to_core_array<pvcop::db::index_t>();
	auto& array_in_2_native = array_in_2.to_core_array<pvcop::db::index_t>();
	auto& array_in_3_native = array_in_3.to_core_array<pvcop::db::index_t>();

	size_t count = 0;
	for (size_t v = 0; v <= DISTINCT_VALUES; v++) {
		for (size_t i = 0; i < v; i++) {
			array_in_1_native[count] = v;
			array_in_2_native[count] = i;
			array_in_3_native[count] = 1;
			tmp_sel[count] = i != 0;

			count++;
		}
	}

	// Shuffle values for more fun !
	std::default_random_engine engine1(0);
	std::default_random_engine engine2(0);
	std::default_random_engine engine3(0);

	std::shuffle(array_in_1_native.begin(), array_in_1_native.end(), engine1);
	std::shuffle(array_in_2_native.begin(), array_in_2_native.end(), engine2);
	std::shuffle(tmp_sel.begin(), tmp_sel.end(), engine3);

	std::copy(tmp_sel.begin(), tmp_sel.end(), out_sel.begin());

	auto start = std::chrono::system_clock::now();

	// Check with all different values on column 2
	pvcop::db::algo::count_by(array_in_1, array_in_2, array_out_1, array_out_2, out_sel);

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

	// Check with all same values on column 2
	pvcop::db::algo::count_by(array_in_1, array_in_3, array_out_3, array_out_4, out_sel);

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "countby with " << size << " elements took: " << diff.count() << " sec"
	          << std::endl;

	auto& array_out_1_native = array_out_1.to_core_array<pvcop::db::index_t>();
	auto& array_out_2_native = array_out_2.to_core_array<pvcop::db::index_t>();
	auto& array_out_4_native = array_out_4.to_core_array<pvcop::db::index_t>();

	pvcop::db::indexes indexes = array_out_1.parallel_sort();
	auto& sorted = indexes.to_core_array();

	PV_VALID(array_out_1_native.size(), array_out_2_native.size());
	for (pvcop::db::index_t i = 0; i < array_out_1_native.size(); i++) {
		PV_VALID(array_out_1_native[sorted[i]],
		         array_out_2_native[sorted[i]] + 1);    // +1 stands for each first unselected line
		PV_VALID(array_out_1_native[sorted[i]], i + 2); // +2 because the lowest distinct value is 2
		PV_VALID(array_out_4_native[sorted[i]], (pvcop::db::index_t)1);
	}

#endif
}
