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

#include <pvcop/db/write_dict.h>
#include <pvcop/db/read_dict.h>

#include <fstream>

/**
 * This bench program aims to measure time to save write_dict with a one million unique URL data
 * set.
 *
 * note: using a read_dict or using the string_set class from the map_index test/bench programs
 * do not change this program run time.
 */
int main()
{
	pvcop::db::read_dict rdict(TESTS_FILES_DIR "/proxy_sample/uniq-uri-1m.txt");

	{
		pvcop::db::write_dict wdict;

		for (const auto key : rdict) {
			wdict.insert(key);
		}

		auto start = std::chrono::system_clock::now();
		wdict.save(std::filesystem::temp_directory_path().string() +  "/db_write_dict_perf_save.dict");
		auto end = std::chrono::system_clock::now();

		std::chrono::duration<double> diff = end - start;
		std::cout << diff.count();
	}

	return 0;
}
