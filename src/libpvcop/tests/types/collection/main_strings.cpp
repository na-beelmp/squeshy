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

#include "export_csv.h"

#include <pvcop/utils/filesystem.h>

#include <common/squey_assert.h>

#include <filesystem>

using namespace pvcop;

static constexpr const char* collection_path = TESTS_FILES_DIR "/pvcop_collection_with_strings";
static constexpr const char* imported_collection_path =
    TESTS_FILES_DIR "/pvcop_collector_with_strings.csv";
static const std::string exported_collection_path =
    std::filesystem::temp_directory_path().string() +  "/pvcop_collection_with_strings.csv";

int main()
{
	if (export_csv(collection_path, exported_collection_path.c_str(), true) != 0) {
		return 1;
	}

	bool equal =
	    pvcop::filesystem::compare_files(imported_collection_path, exported_collection_path.c_str());

	PV_ASSERT_VALID(equal);
}
