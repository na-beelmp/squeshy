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

#include <pvcop/db/read_dict.h>

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

using string_set = std::vector<std::string>;

string_set strings = {"Lorem",       "ipsum",        "dolor",      "sit",       "amet",
                      "consectetur", "adipiscing",   "elit",       "sed",       "do",
                      "eiusmod",     "tempor",       "incididunt", "ut",        "labore",
                      "et",          "dolore",       "magna",      "aliqua",    "Ut",
                      "enim",        "ad",           "minim",      "veniam",    "quis",
                      "nostrud",     "exercitation", "ullamco",    "laboris",   "nisi",
                      "aliquip",     "ex",           "ea",         "commodo",   "consequat",
                      "Duis",        "aute",         "irure",      "in",        "reprehenderit",
                      "voluptate",   "velit",        "esse",       "cillum",    "eu",
                      "fugiat",      "nulla",        "pariatur",   "Excepteur", "sint",
                      "occaecat",    "cupidatat",    "non",        "proident",  "sunt",
                      "culpa",       "qui",          "officia",    "deserunt",  "mollit",
                      "anim",        "id",           "est",        "laborum"};

int main()
{
	std::string filename(std::filesystem::temp_directory_path().string() +  "/read_dict_test.dict");
	bool has_error = false;

	/* saving dictionary
	 */
	{
		std::ofstream ofs(std::filesystem::path(filename), std::ios::binary);

		size_t string_count = strings.size() - 1;

		for (size_t i = 0; i < string_count; ++i) {
			ofs << strings[i] << '\n';
		}
		ofs << strings[string_count];
	}

	pvcop::db::read_dict rd1;

	/* test for ::load(...)
	 */
	rd1.load(filename);

	if (rd1.size() != strings.size()) {
		pvlogger::error() << "rd1::size() returns " << rd1.size() << " instead of "
		                  << strings.size() << std::endl;
		return 1;
	}

	/* test loaded content, it relies on the fact that read_dict::key(...) is
	 * enough trivial to be valid
	 */

	for (size_t i = 0; i < rd1.size(); ++i) {
		const auto& k = rd1.key(i);
		if (strings[i] != k) {
			pvlogger::error() << "rd1.key(" << i << ") returns '" << k << "' instead of '"
			                  << strings[i] << std::endl;
			has_error = true;
		}
	}

	if (has_error) {
		return 1;
	}

	/* no need to test the "file" constructor as it implicitly calls ::load(...)
	 */

	{
		/* test of ::create(...)
		 */
		pvcop::db::read_dict rd2;

		rd2.create(strings);

		if (rd2.size() != strings.size()) {
			pvlogger::error() << "rd2::size() returns " << rd2.size() << " instead of "
			                  << strings.size() << std::endl;
			return 1;
		}

		for (size_t i = 0; i < rd2.size(); ++i) {
			const auto& k = rd2.key(i);
			if (strings[i] != k) {
				pvlogger::error() << "rd2.key(" << i << ") returns '" << k << "' instead of '"
				                  << strings[i] << std::endl;
				has_error = true;
			}
		}

		if (has_error) {
			return 1;
		}
	}

	/* no need to test the "string set" constructor as it implicitly calls ::create(...)
	 */

	/* test of iterators
	 */
	if (std::equal(rd1.begin(), rd1.end(), strings.begin()) == false) {
		pvlogger::error() << "error comparing using iterators" << std::endl;
		return 1;
	}

	/* test of ::index(...)
	 */
	for (size_t i = 0; i < strings.size(); ++i) {
		auto index = rd1.index(strings[i].data());

		if (index != i) {
			pvlogger::error() << "::index('" << strings[i] << "') returns " << index
			                  << " instead of" << i << std::endl;
			has_error = true;
		}
	}

	if (has_error) {
		return 1;
	}

	/* test of ::find(...)
	 */
	for (size_t i = 0; i < strings.size(); ++i) {
		auto& value = rd1.find(strings[i].data());

		if ((value.second != i) || (strings[i] != value.first)) {
			pvlogger::error() << "find('" << strings[i] << "') returns ('" << value.first << "',"
			                  << value.second << ") instead of ('" << strings[i] << "'," << i << ")"
			                  << std::endl;
			has_error = true;
		}
	}

	if (has_error) {
		return 1;
	}

	/* test of ::exist(...)
	 */
	if (rd1.exist(strings[0].data()) != true) {
		pvlogger::error() << "exist('" << strings[0] << "') returns false instead of true"
		                  << std::endl;
		return 1;
	}

	std::string bad_word("ipso");

	if (rd1.exist(bad_word.data()) != false) {
		pvlogger::error() << "exist('" << bad_word << "') returns true instead of false"
		                  << std::endl;
		return 1;
	}

	return 0;
}
