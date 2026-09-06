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
#include <filesystem>

#include <common/squey_assert.h>

#ifdef PVCOP_BENCH
static constexpr size_t ELEM_SIZE = 100000000;
#else
static constexpr size_t ELEM_SIZE = 100000;
#endif

static constexpr size_t DISTINCT_VALUES_RATIO = 10000;
static constexpr size_t DEFAULT_VALUE = 7;
static constexpr size_t DISTINCT_VALUES_COUNT = ELEM_SIZE / DISTINCT_VALUES_RATIO;

int main()
{
	using type1_t = uint64_t;
	std::string type1 = "number_uint64";

	using type2_t = double;
	std::string type2 = "number_double";

	pvcop::db::array array_out_1;
	pvcop::db::array array_out_2;

	pvcop::db::array array_in_1(type1, ELEM_SIZE);
	pvcop::db::array array_in_2(type2, ELEM_SIZE);

	auto& array_in_1_native = array_in_1.to_core_array<type1_t>();
	auto& array_in_2_native = array_in_2.to_core_array<type2_t>();

	for (size_t i = 0; i < ELEM_SIZE; i++) {
		array_in_1_native[i] = i % DISTINCT_VALUES_COUNT;
	}
	std::fill(array_in_2_native.begin(), array_in_2_native.end(), DEFAULT_VALUE);

	auto start = std::chrono::system_clock::now();

	pvcop::db::algo::sum_by(array_in_1, array_in_2, array_out_1, array_out_2);

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "sumby with " << ELEM_SIZE << " elements took: " << diff.count() << " sec" << std::endl;

	bool sizes_ok =
	    array_out_1.size() == array_out_2.size() && array_out_1.size() == DISTINCT_VALUES_COUNT;
	PV_ASSERT_VALID(sizes_ok);

	auto& array_out_1_native = array_out_1.to_core_array<type1_t>();

	std::sort(array_out_1_native.begin(), array_out_1_native.end());
	bool group = std::unique(array_out_1_native.begin(), array_out_1_native.end()) ==
	             array_out_1_native.end();
	group &= *array_out_1_native.begin() == 0;
	group &= *std::prev(array_out_1_native.end()) == DISTINCT_VALUES_COUNT - 1;
	// TODO : implement core::array::front() and core::array::back()

	PV_ASSERT_VALID(group);

	const auto& array_out_2_native = array_out_2.to_core_array<type2_t>();

	bool sum_ok =
	    std::all_of(array_out_2_native.begin(), array_out_2_native.end(),
	                [&](type2_t val) { return val == (DISTINCT_VALUES_RATIO * DEFAULT_VALUE); });
	PV_ASSERT_VALID(sum_ok);
#endif

	{
		const char* csv_file_path = TESTS_FILES_DIR "/invalid_sumby.csv";
		const char* csv_format_path = TESTS_FILES_DIR "/invalid_sumby.csv.formatters";
		const std::string collector_path =
		    std::filesystem::temp_directory_path().string() + "/collector_test_sum_by";

		import(csv_file_path, pvcop::formatter_desc_list(csv_format_path), collector_path.c_str());

		pvcop::collection collection(collector_path.c_str());
		pvcop::db::array col1 = collection.column(0);
		pvcop::db::array col2 = collection.column(1);

		pvcop::db::algo::sum_by(col1, col2, array_out_1, array_out_2);

		pvcop::db::indexes indexes = array_out_1.parallel_sort();
		auto& sorted = indexes.to_core_array();

		const char* sumby_output_path = TESTS_FILES_DIR "/invalid_sumby_output.csv";
		const char* sumby_output_format = TESTS_FILES_DIR "/invalid_sumby_output.csv.formatters";
		const std::string collector_path2 =
		    std::filesystem::temp_directory_path().string() + "/collector_test_sum_by2";
		import(sumby_output_path, pvcop::formatter_desc_list(sumby_output_format), collector_path2.c_str());

		pvcop::collection output_collection(collector_path2);
		pvcop::db::array output_col1 = output_collection.column(0);
		pvcop::db::array output_col2 = output_collection.column(1);

		for (size_t i = 0; i < array_out_1.size(); i++) {
			pvlogger::info() << array_out_1.at(i) << " " << array_out_2.at(i) << std::endl;
			PV_VALID(array_out_1.at(sorted[i]), output_col1.at(i));
			PV_VALID(array_out_2.at(sorted[i]), output_col2.at(i));
		}
	}

	{
		const char* csv_file_path = TESTS_FILES_DIR "/invalid_sumby_double.csv";
		const char* csv_format_path = TESTS_FILES_DIR "/invalid_sumby_double.csv.formatters";
		std::string collector_path =
		    std::filesystem::temp_directory_path().string() + "/collector_test_sum_by3";

		import(csv_file_path, pvcop::formatter_desc_list(csv_format_path), collector_path.c_str());

		pvcop::collection collection(collector_path);
		pvcop::db::array col1 = collection.column(0);
		pvcop::db::array col2 = collection.column(1);

		pvcop::core::memarray<bool> input_sel(col1.size());
		pvcop::db::algo::subselect(col1, col1.to_array(std::vector<std::string>{"237"}), {},
		                           input_sel);

		pvcop::db::array array_out_1;
		pvcop::db::array array_out_2;
		pvcop::db::algo::sum_by(col1, col2, array_out_1, array_out_2, input_sel);

		PV_VALID(array_out_1.size(), (size_t)1);
		PV_VALID(array_out_2.size(), (size_t)1);
		PV_VALID(array_out_1.at(0), std::string("237"));
		PV_ASSERT_VALID(std::stof(array_out_2.at(0)) >
		                54.89); // take floating precision into account
		PV_ASSERT_VALID(std::stof(array_out_2.at(0)) < 54.91);
	}
}
