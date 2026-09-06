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
#include <pvcop/collector.h>
#include <pvcop/collection.h>
#include <types/common/import_pipeline.h>
#include <common/squey_assert.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <random>

#include <filesystem>

#ifdef PVCOP_BENCH
static constexpr size_t ELEM_SIZE = 200000000; //!< Number of data elements
#else
static constexpr size_t ELEM_SIZE = 1000000; //!< Number of data elements
#endif

int main()
{
	// Initialize random
	std::random_device rd;
	std::mt19937 g(rd());

	// Generate initial data
	std::uniform_int_distribution<uint64_t> dis;
	pvcop::db::array array("number_uint64", ELEM_SIZE);
	auto& native_array = array.to_core_array<uint64_t>();
	std::generate(native_array.begin(), native_array.end(), [&dis, &g]() { return dis(g); });

	auto start = std::chrono::system_clock::now();

	// Check indexed-sort
	pvcop::db::indexes indexes = array.parallel_sort();
	auto& native_indexes = indexes.to_core_array();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "Time to sort " << ELEM_SIZE << " elements: " << diff.count() << " sec" << std::endl;

	bool res =
	    std::is_sorted(native_indexes.begin(), native_indexes.end(), [&](uint64_t v1, uint64_t v2) {
		    return native_array[v1] <= native_array[v2];
		});

	PV_ASSERT_VALID(res);

	// Check in-place sort
	pvcop::db::groups groups;
	pvcop::db::extents extents;

	array.group(groups, extents);
	array.parallel_sort(extents);

	const auto& sorted_extents = extents.to_core_array();
	PV_ASSERT_VALID(
	    std::is_sorted(sorted_extents.begin(), sorted_extents.end(), [&](uint32_t e1, uint32_t e2) {
		    return native_array[e1] < native_array[e2];
		}));

	// Check if exception is properly thrown for inplace-sort
	pvcop::db::array small_array("number_uint64", ELEM_SIZE - 1);
	try {
		small_array.parallel_sort(indexes);
		PV_ASSERT_VALID(false);
	} catch (pvcop::db::exception::size_mismatch_error const& e) {
		PV_ASSERT_VALID(true);
	}

	const char* csv_file_path = TESTS_FILES_DIR "/invalid_values.csv";
	const char* csv_format_path = TESTS_FILES_DIR "/invalid_values.csv.formatters";
	std::string collector_path =
	    std::filesystem::temp_directory_path().string() + "/collector_test_db_array_sort";

	import(csv_file_path, pvcop::formatter_desc_list(csv_format_path), collector_path.c_str());

	pvcop::collection collection(collector_path);

	pvcop::db::array col1 = collection.column(0);

	pvcop::db::indexes sorted_array = col1.parallel_sort();

	const auto& sorted = sorted_array.to_core_array();
	const auto& core_col1 = col1.to_core_array<int32_t>();

	PV_ASSERT_VALID(col1.has_invalid() and sorted.size() == col1.size());

	for (size_t i = 1; i < col1.size(); i++) {
		if (col1.is_valid(sorted[i - 1]) and col1.is_valid(sorted[i])) {
			PV_ASSERT_VALID(core_col1[sorted[i - 1]] <= core_col1[sorted[i]]);
		} else if (col1.is_valid(sorted[i - 1]) ^ col1.is_valid(sorted[i])) {
			PV_ASSERT_VALID(col1.is_valid(sorted[i - 1]) and not col1.is_valid(sorted[i]));
		} else {
			PV_ASSERT_VALID(col1.at(sorted[i - 1]) <= col1.at(sorted[i]));
		}
	}

#endif // PVCOP_BENCH
}
