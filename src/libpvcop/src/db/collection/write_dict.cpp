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

#include <vector>
#include <fstream>
#include <filesystem>

/*****************************************************************************
 * pvcop::db::write_dict::save
 *****************************************************************************/

void pvcop::db::write_dict::save(const std::string& filename) const
{
	size_t value_count = _index.size();

	if (value_count == 0) {
		// No row collected so we don't have to save data.
		return;
	}

	std::vector<const char*> ordered_strings(value_count);

	for (const auto& entry : _index) {
		ordered_strings[entry.second] = entry.first.value();
	}

	std::ofstream file{std::filesystem::path(filename), std::ios::binary | std::ios_base::out};

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	if (!file.is_open()) {
		throw std::ios::failure(std::string("can not open file '" + filename + "' for writing"));
	}

	/* need to save the N-1 first entries with a trailing new-line
	 */
	value_count -= 1;

	for (size_t i = 0; i < value_count; ++i) {
		file << ordered_strings[i] << '\n';
	}

	/* and the last one without new-line.
	 */
	file << ordered_strings[value_count];
}
