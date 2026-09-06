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
#include <pvcop/types/mac_address.h>
#include <pvcop/formatter_desc_list.h>
#include <pvcop/collection.h>

#include <common/squey_assert.h>

#include "common/import_pipeline.h"

#include <vector>
#include <cstring>
#include <filesystem>

#define BUFFER_LEN 1024

const struct {
	const char* input;
	const char* output;
	uint64_t value;
	bool result;
} dataset[] = {
    // the 3 valid forms (with case sensitiveness)
    {"00:11:22:33:44:FF", "00:11:22:33:44:FF", 0x0011223344FF, true},
    {"00-11-22-33-44-ff", "00:11:22:33:44:FF", 0x0011223344FF, true},
    {"0011.2233.44Ff", "00:11:22:33:44:FF", 0x0011223344FF, true},
    // a valid but not supported EUI-64
    {"00:11:22:33:44:55:66:77", "", 0x0, false},
    // a truncated MAC address
    {"00:11:22:33", "", 0x0, false},
    // ill-formed MAC addresses
    {"0:11:22:33:44:55", "", 0x0, false},
    {"00:111:22:33:44:55", "", 0x0, false},
    {"00:11:22:33:44:557", "", 0x0, false},
    // MAC addresses with garbage
    {"00:11:22:33:44:55.", "", 0x0, false},
    {"00:s11:22:33:44:55", "", 0x0, false},
    {"a00:11:22:33:g44:55", "", 0x0, false}};

static constexpr const char* CSV_FILE_PATH = TESTS_FILES_DIR "/mac_address.csv";
static constexpr const char* OUTPUT_FILE_PATH = TESTS_FILES_DIR "/mac_address.sorted";
static const std::string COLLECTOR_PATH = std::filesystem::temp_directory_path().string() +  "/mac_address.pvcop";

void test_formatting()
{
	pvcop::types::formatter_mac_address maf;

	std::vector<char> buffer(BUFFER_LEN);

	for (const auto& data : dataset) {
		uint64_t value;
		const bool conv_res = maf.from_string(data.input, &value, 0);

		PV_VALID(conv_res, data.result, "input", data.input);

		if (data.result) {
			PV_VALID(value, data.value, "input", data.input);

			maf.to_string(buffer.data(), BUFFER_LEN, &value, 0);
			PV_VALID(strcmp(buffer.data(), data.output), 0, "input", data.input);
		}
	}
}

void test_sort()
{
	pvcop::formatter_desc_list formatter_descs;
	formatter_descs.emplace_back("mac_address", "");

	size_t failing_import = import(CSV_FILE_PATH, formatter_descs, COLLECTOR_PATH.c_str());
	PV_VALID(failing_import, (size_t)0);

	pvcop::collection collection(COLLECTOR_PATH.c_str());
	pvcop::db::array column = collection.column(0);

	pvcop::db::indexes indexes = column.parallel_sort();
	const auto& sorted = indexes.to_core_array();

	std::ifstream wanted_macs(std::filesystem::path{OUTPUT_FILE_PATH});
	std::string wanted_mac;

	for (size_t i = 0; i < column.size(); ++i) {
		wanted_macs >> wanted_mac;
		PV_VALID(column.at(sorted[i]), wanted_mac, "i", i);
	}
}

int main()
{
	test_formatting();
	test_sort();

	return 0;
}
