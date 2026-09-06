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
#include <pvcop/types/factory.h>

#include <common/squey_assert.h>

#include "common/import_pipeline.h"

#include <pvlogger.h>

#include <algorithm>
#include <filesystem>

#ifdef PVCOP_BENCH
const char* csv_file_path = TESTS_FILES_DIR "/sort_bench_15000000_50000_1_40.txt";
#else
const char* csv_file_path = TESTS_FILES_DIR "/shuffled_alphabet.txt";
#endif

using namespace pvcop;

int main(int argc, char** argv)
{
	const char* csv_format_path = nullptr;
	std::string collector_path;

	if (argc != 3) {
		pvlogger::info() << "usage: " << basename(argv[0])
		                 << " <string_file> <collector_path>. (using default values)" << std::endl;

		collector_path =
		    std::filesystem::temp_directory_path().string() + "/collector_test_types_sort";
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

	pvcop::db::array column = collection.column(0);

	auto start = std::chrono::system_clock::now();
	pvcop::db::indexes indexes = column.parallel_sort();
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "sorting " << column.size() << " strings took " << diff.count() << " sec"
	          << std::endl;

	const auto& core_indexes = indexes.to_core_array();
	bool sorted =
	    std::is_sorted(core_indexes.begin(), core_indexes.end(),
	                   [&](const pvcop::db::index_t& idx1, const pvcop::db::index_t& idx2) {
		                   return column.at(idx1) < column.at(idx2);
		               });

	PV_ASSERT_VALID(sorted);
#endif
}
