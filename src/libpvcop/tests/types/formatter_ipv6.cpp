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

#include <pvcop/pvcop.h>
#include <pvcop/types/ipv6.h>
#include <pvcop/collector.h>
#include <pvcop/collection.h>
#include <pvcop/types/factory.h>

#include <filesystem>

#include "formatter_check.h"

#include "common/import_pipeline.h"

static constexpr const char* CSV_FILE_PATH = TESTS_FILES_DIR "/ipv4_ipv6.csv";
static constexpr const char* OUTPUT_FILE_PATH = TESTS_FILES_DIR "/ipv4_ipv6.sorted";
static std::string COLLECTOR_PATH = std::filesystem::temp_directory_path().string() +  "/ipv4_ipv6.pvcop";

int main()
{
	/**
	 * Test formatting
	 */
	std::cout << "formatter_ipv6:" << std::endl;
	pvcop::types::formatter_ipv6 ipv6f("");
	bench_formatter<pvcop::db::uint128_t>(ipv6f);

	/**
	 * Test sorting
	 */
	pvcop::formatter_desc_list formatter_descs;
	formatter_descs.emplace_back(pvcop::formatter_desc("ipv6", ""));
	formatter_descs.emplace_back(pvcop::formatter_desc("number_uint32", ""));

	size_t failing_import = import(CSV_FILE_PATH, formatter_descs, COLLECTOR_PATH.c_str());

	if (failing_import != 0) {
		pvlogger::error() << "Error while processing some rows" << std::endl;
		return 2;
	}

	pvcop::collection collection(COLLECTOR_PATH.c_str());
	pvcop::db::array column = collection.column(0);

	pvcop::db::indexes indexes = column.parallel_sort();
	const auto& sorted = indexes.to_core_array();

	std::ifstream wanted_ips(std::filesystem::path{OUTPUT_FILE_PATH});
	std::string wanted_ip;
	for (size_t i = 0; i < column.size(); i++) {
		wanted_ips >> wanted_ip;

		PV_VALID(column.at(sorted[i]), wanted_ip);
	}
}
