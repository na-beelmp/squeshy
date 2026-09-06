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

		csv_file_path = TESTS_FILES_DIR "/long_fields.csv";
		csv_format_path = TESTS_FILES_DIR "/long_fields.csv.formatters";
		collector_path = std::filesystem::temp_directory_path().string() +  "/collector_test_long_fields";
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

	std::ifstream ref(std::filesystem::path{csv_file_path});
	std::stringstream ref_buf;
	ref_buf << ref.rdbuf();

	std::string res = col1.at(0) + "," + col2.at(0) + "\n";

	PV_VALID(ref_buf.str(), res);

	return 0;
}
