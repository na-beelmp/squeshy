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
#include <pvcop/types/factory.h>

#include "types/common/import_pipeline.h"

#include <common/squey_assert.h>

#include <pvlogger.h>

#include <unistd.h>
#include <memory>
#include <filesystem>
using namespace pvcop;

int main(int argc, char** argv)
{
	const char* csv_file_path = nullptr;
	const char* csv_format_path = nullptr;
	std::string collector_path;

	if (argc != 4) {
		pvlogger::info() << "usage: " << basename(argv[0])
		                 << " <csv_file> <csv_format_file> (using default values)" << std::endl;

		csv_file_path = TESTS_FILES_DIR "/shuffled_alphabet.txt";
		collector_path =
		    std::filesystem::temp_directory_path().string() + "/collector_test_sortable_distinct";
	} else {
		csv_file_path = argv[1];
		collector_path = argv[2];
	}

	formatter_desc_list formatter_descs;
	formatter_descs.emplace_back(formatter_desc("string", ""));
	if (formatter_descs == false) {
		pvlogger::error() << "error loading formatter list '" << csv_format_path << "'"
		                  << std::endl;
		return 1;
	}

	size_t failing_import = import(csv_file_path, formatter_descs, collector_path.c_str());

	if (failing_import != 0) {
		pvlogger::error() << "Error while processing some rows" << std::endl;
		return 2;
	}

	collection collection(collector_path);

	pvcop::db::array array_in = collection.column(0);
	pvcop::db::array distinct_array;

	pvcop::db::algo::distinct(array_in, distinct_array);

	// Check that sort is working
	pvcop::db::indexes indexes = distinct_array.parallel_sort();
	const auto& native_indexes = indexes.to_core_array();

	bool sorted = std::is_sorted(native_indexes.begin(), native_indexes.end(),
	                             [&](string_index_t idx1, string_index_t idx2) {
		                             return distinct_array.at(idx1) < distinct_array.at(idx2);
		                         });
	PV_ASSERT_VALID(sorted);
}
