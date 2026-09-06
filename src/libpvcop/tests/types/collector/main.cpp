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
#include <pvcop/types/factory.h>

#include "types/common/import_pipeline.h"

#include <pvlogger.h>

#include <unistd.h>
#include <memory>
#include <filesystem>
using namespace pvcop;

/*
Bench realized on a CSV file containing 161,051,000 lines x 9 columns (14GB) of random values of the
following types:
[uint8, uint16, uint32, uint64, int8, int16, int32, int64, float]

Hardware configuration:
        dual Intel(R) Xeon(R) CPU X5650  @ 2.67GHz (12 cores)
        192 Gio DDR3 RAM

|fields                            | Bandwidth  | Comment                                         |
|----------------------------------|------------|-------------------------------------------------|
|store by row / write by column    | 525MB/s    |                                                 |
|store by column / write by row    | 668MB/s    |                                                 |
|store by row / write by row       | 789MB/s    |                                                 |
|store by column / write by column | 862MB/s    |                                                 |
|store by column / write by column | 915MB/s    | using tbb::scalable_allocator for sink::fields* |

Context:
 - read/write done on a tmpfs (and not on a ramfs because of this bug:
https://sourceware.org/bugzilla/show_bug.cgi?id=15661)
 - CPU gouvernor set to "performance"
 - droping system cache prior to BENCH_START ("# sync && echo 3 > /proc/sys/vm/drop_caches")
 - flushing data before BENCH_END ("sync()")
 - Bandwidth is computed on an average of 10 runs
*/

int main(int argc, char** argv)
{
	const char* csv_file_path = nullptr;
	const char* csv_format_path = nullptr;
	std::string collector_path;

	if (argc != 4) {
#ifndef PVCOP_BENCH
		pvlogger::info() << "usage: " << basename(argv[0])
		                 << " <csv_file> <csv_format_file> <collector_path>. (using default values)"
		                 << std::endl;
#endif

		csv_file_path = TESTS_FILES_DIR "/pvcop_collector_with_strings.csv";
		csv_format_path = TESTS_FILES_DIR "/pvcop_collector_with_strings.formatters";
		collector_path =
		    std::filesystem::temp_directory_path().string() + "/collector_test_types_collector";
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

	auto start = std::chrono::system_clock::now();

	size_t failing_import = 0;

	failing_import = import(csv_file_path, formatter_descs, collector_path.c_str());

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	if (failing_import != 0) {
		pvlogger::error() << "Error while processing some rows" << std::endl;
		return 2;
	} else {
		pvlogger::info() << "import on file '" << csv_file_path << "' and format '"
		                 << csv_format_path << "' took " << diff.count() << " sec" << std::endl;
	}
#endif
	return 0;
}
