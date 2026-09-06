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

#include <pvcop/collection.h>
#include <pvcop/db/exceptions/invalid_collection.h>

#include <common/squey_assert.h>

#include <filesystem>

using namespace pvcop;

static constexpr const char* collection_path =
    TESTS_FILES_DIR "/pvcop_collection"; // pvcop_collection_5000000";
static constexpr const char* imported_collection_path =
    TESTS_FILES_DIR "/pvcop_collector.csv"; // pvcop_collector_5000000.csv";
static std::string exported_collection_path = std::filesystem::temp_directory_path().string() +  "/pvcop_collection.csv";

/**
 * row count per chunk for 500,000 rows (computed on 10 runs on pbrunet's computer)
 * It is a 4 core with hyperthreads (8 threads)
 *
 * | row count  | mean time (sec) | std             | median time (sec) |
 * |------------|-----------------|-----------------|-------------------|
 * | 2048 (seq) | 2.452837        | 0.0430583060628 | 2.44032           |
 * |------------|-----------------|-----------------|-------------------|
 * | 1          | 6.930885        | 0.0608391578262 | 6.89811           |
 * | 2          | 2.465518        | 0.0847302759113 | 2.4305            |
 * | 4          | 1.408504        | 0.0380459843347 | 1.398345          |
 * | 8          | 1.130774        | 0.0185103426224 | 1.124195          |
 * | 16         | 1.081529        | 0.0168316514044 | 1.080005          |
 * | 32         | 0.941687        | 0.0128567725499 | 0.939981          |
 * | 64         | 0.862586        | 0.0402785228652 | 0.8495            |
 * | 128        | 0.8071806       | 0.0159925185584 | 0.8026175         |
 * | 256        | 0.7757664       | 0.0147881855899 | 0.7731655         |
 * | 512        | 0.7565198       | 0.0142396915894 | 0.7530005         |
 * | 1024       | 0.7502831       | 0.016050067417  | 0.7509475         |
 * | 2048       | 0.7631927       | 0.0218234029521 | 0.7643575         |
 * | 4096       | 0.7607935       | 0.0222536430467 | 0.7625125         |
 *
 * On 5,000,000 rows, results are proportional
 *
 * | row count  | mean time (sec) | std             | median time (sec) |
 * |------------|-----------------|-----------------|-------------------|
 * | 1024       | 7.779638        | 0.484936176064  | 7.604655          |
 * | 2048       | 7.5301          | 0.227510531273  | 7.45966           |
 * | 4096       | 7.497841        | 0.0836116773483 | 7.481285          |
 */

int main()
{
	{
		/* check that db::exception::invalid_collection is thrown on a nonexistent directory
		 */
		try {
			pvcop::collection(std::filesystem::temp_directory_path().string() +  "/missing_dir");
			return 1;
		} catch (pvcop::db::exception::invalid_collection&) {
			// catching it with no code to accept it
		}
	}

	if (export_csv(collection_path, exported_collection_path.c_str(), true) != 0) {
		return 1;
	}

	pvlogger::info() << imported_collection_path << " " << exported_collection_path << std::endl;
	bool equal =
	    pvcop::filesystem::compare_files(imported_collection_path, exported_collection_path.c_str());

	PV_ASSERT_VALID(equal);
}
