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

#include <pvcop/collector.h>
#include <pvcop/collection.h>
#include <pvcop/db/algo.h>

#include <types/common/import_pipeline.h>

#include <common/squey_assert.h>

#include <pvlogger.h>

#include <libgen.h>
#include <filesystem>

using namespace pvcop;

int main(int argc, char** argv)
{
	const char* csv_file_path = nullptr;
	const char* csv_format_path = nullptr;
	std::string collector_path;

	if (argc != 4) {
		pvlogger::info() << "usage: " << basename(argv[0])
		                 << " <csv_file> <csv_format_file> <collector_path>. (using default values)"
		                 << std::endl;

		csv_file_path = TESTS_FILES_DIR "/invalid_values.csv";
		csv_format_path = TESTS_FILES_DIR "/invalid_values.csv.formatters";
		collector_path = std::filesystem::temp_directory_path().string() +  "/collector_test_invalid_values";
	} else {
		csv_file_path = argv[1];
		csv_format_path = argv[2];
		collector_path = argv[3];
	}

	formatter_desc_list formatter_descs(csv_format_path);
	if (formatter_descs == false) {
		pvlogger::error() << "error loading formatter list '" << csv_format_path << "'"
		                  << std::endl;
		return 1;
	}

	import(csv_file_path, formatter_descs, collector_path.c_str());

	pvcop::collection collection(collector_path);

	const db::array& col1 = collection.column(0);
	const db::array& col2 = collection.column(1);
	const db::array& col3 = collection.column(2);
	const db::array& col4 = collection.column(3);
	const db::array& col5 = collection.column(4);

	PV_VALID(col1.has_invalid(), pvcop::db::INVALID_TYPE::INVALID);
	PV_VALID(col2.has_invalid(), pvcop::db::INVALID_TYPE::NONE);
	PV_VALID(col3.has_invalid(), pvcop::db::INVALID_TYPE::EMPTY);
	PV_VALID(col4.has_invalid(), (pvcop::db::INVALID_TYPE)(pvcop::db::INVALID_TYPE::EMPTY |
	                                                       pvcop::db::INVALID_TYPE::INVALID));

	/**
	 * Check that invalid values are properly stored
	 *
	 * invalid values are located at indexes 0, 5, 10, 15, 20, etc...
	 */
	PV_VALID(col1.type(), std::string("number_int32"));
	for (size_t row = 0; row < col1.size(); row++) {
		bool invalid = not(row % 5);
		PV_VALID(col1.at(row), std::string(invalid ? "test" : "") + std::to_string(row));
		PV_VALID(col1.is_valid(row), not invalid);
	}

	/**
	 * Check that slicing keeps invalid values
	 */
	const db::array& slice = col1.slice(64, 64);
	PV_VALID(slice.type(), col1.type());
	PV_VALID(slice.size(), (size_t)64);
	const db::selection& inv_sel_slice = slice.invalid_selection();
	PV_ASSERT_VALID(inv_sel_slice == true);
	PV_VALID(inv_sel_slice.size(), (size_t)64);
	for (size_t row = 0; row < slice.size(); row++) {
		bool invalid = (row == 1 or (not((row + 4) % 5) and row > 1));
		PV_VALID(slice.at(row),
		         (invalid ? std::string("test") : std::string("")) + std::to_string(row + 64));
		PV_VALID(slice.is_valid(row), not invalid);
		PV_VALID(inv_sel_slice[row], invalid);
	}

	/**
	 * Check invalid values conversion (to_array) and filtering (subselect)
	 */
	std::vector<std::string> unconvertable_values;
	core::memarray<bool> out_sel(col1.size());
	std::vector<std::string> values{"1",
	                                "127",

	                                "test0",
	                                "test125",

	                                "unconvertable",
	                                ""};

	const db::array& converted_array = db::algo::to_array(col1, values, &unconvertable_values);
	db::algo::subselect(col1, converted_array, {}, out_sel);

	PV_VALID(core::algo::bit_count(out_sel), (size_t)4);
	PV_VALID(converted_array.size(), (size_t)4);
	PV_VALID(converted_array.at(0), std::string("1"));
	PV_VALID(converted_array.is_valid(0), true);
	PV_VALID(converted_array.at(1), std::string("127"));
	PV_VALID(converted_array.is_valid(1), true);
	PV_VALID(converted_array.at(2), std::string("test0"));
	PV_VALID(converted_array.is_valid(2), false);
	PV_VALID(converted_array.at(3), std::string("test125"));
	PV_VALID(converted_array.is_valid(3), false);

	PV_VALID(unconvertable_values.size(), (size_t)2);
	PV_VALID(unconvertable_values[0], std::string("unconvertable"));
	PV_VALID(unconvertable_values[1], std::string(""));

	/**
	 * Check group with invalid values
	 */
	pvcop::db::groups groups;
	pvcop::db::extents extents;
	col1.group(groups, extents);
	const auto& core_extents = extents.to_core_array();

	PV_VALID(extents.size(), (size_t)128);
	PV_VALID((bool)extents.has_invalid(), true);
	PV_VALID(extents.invalid_count(), (size_t)26);
	PV_VALID(extents.valid_count(), (size_t)102);

	for (size_t i = 0; i < extents.size(); i++) {
		PV_VALID(extents.is_valid(i), col1.is_valid(core_extents[i]));
		PV_VALID(col1.at(core_extents[i]),
		         std::string(extents.is_valid(i) ? "" : "test") + std::to_string(core_extents[i]));
	}

	/**
	 * Check distinct with invalid values
	 */
	{
		pvcop::db::array distinct_array;
		pvcop::db::array distinct_histogram;
		pvcop::db::algo::distinct(col1, distinct_array, distinct_histogram);
		PV_VALID(distinct_array.size(), (size_t)128);
		PV_VALID((bool)distinct_array.has_invalid(), true);
		PV_VALID(distinct_array.invalid_count(), (size_t)26);
		PV_VALID(distinct_array.valid_count(), (size_t)102);
	}

	/**
	 * Check distinct with invalid values on a float column
	 */
	{
		pvcop::db::array distinct_array;
		pvcop::db::array distinct_histogram;
		pvcop::db::algo::distinct(col5, distinct_array, distinct_histogram);
		PV_VALID(distinct_array.size(), (size_t)126);
		PV_VALID((bool)distinct_array.has_invalid(), true);
		PV_VALID(distinct_array.invalid_count(), (size_t)3); // there are three different invalid values
		PV_VALID(distinct_array.valid_count(),
		         (size_t)123); // there are two couples of float with the same values
		PV_ASSERT_VALID(col5.at(125) == "test1");
		PV_ASSERT_VALID(col5.at(126) == "test2");
		PV_ASSERT_VALID(col5.at(127) == "test3");
		pvcop::db::indexes indexes = distinct_array.parallel_sort();
		auto& sorted = indexes.to_core_array();
		PV_ASSERT_VALID(distinct_array.at(sorted[123]) == "test1");
		PV_ASSERT_VALID(distinct_array.at(sorted[124]) == "test2");
		PV_ASSERT_VALID(distinct_array.at(sorted[125]) == "test3");
	}

	/**
	 * Check that invalid selection files are completely written to disk
	 */
	const char* invalid_sel_file_path = TESTS_FILES_DIR "/invalid_sel.csv";
	const char* invalid_sel_format_path = TESTS_FILES_DIR "/invalid_sel.csv.formatters";
	collector_path = std::filesystem::temp_directory_path().string() +  "/collector_test_invalid_sel";

	import(invalid_sel_file_path, formatter_desc_list(invalid_sel_format_path), collector_path.c_str());

	pvcop::collection collection_sel(collector_path);

	PV_ASSERT_VALID((bool)collection_sel.column(0).has_invalid());
	PV_ASSERT_VALID((bool)collection_sel.column(1).has_invalid());
	PV_ASSERT_VALID((bool)collection_sel.column(2).has_invalid());

	/**
	 * Check minmax with invalid values
	 */
	const char* invalid_minmax_file_path = TESTS_FILES_DIR "/invalid_minmax.csv";
	const char* invalid_minmax_format_path = TESTS_FILES_DIR "/invalid_minmax.csv.formatters";
	collector_path = std::filesystem::temp_directory_path().string() +  "/collector_test_invalid_minmax";

	import(invalid_minmax_file_path, formatter_desc_list(invalid_minmax_format_path),
	       collector_path.c_str());

	pvcop::collection collection_minmax(collector_path);

	pvcop::db::array minmax = pvcop::db::algo::minmax(collection_minmax.column(0));

	PV_VALID(minmax.size(), (size_t)2);
	PV_VALID(minmax.at(0), std::string("0"));
	PV_VALID(minmax.at(1), std::string("324.118286"));

	return 0;
}
