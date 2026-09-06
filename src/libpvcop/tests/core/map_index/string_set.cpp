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

#include "string_set.h"

#include <fstream>
#include <stdexcept>

/**
 * Dump file content in a string.
 */
static std::string string_from_file(std::string const& filename)
{
	std::ifstream ifs(std::filesystem::path{filename});

	if (not ifs) {
		throw std::runtime_error("Unknown file");
	}

	return {std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
}

string_set::string_set(std::string const& filename) : _data(string_from_file(filename))
{
	// Beginning of data is the first word.
	_entries.push_back(&_data.front());

	for (size_t end = _data.find_first_of('\n'); end != std::string::npos;
	     end = _data.find_first_of('\n', end + 1)) {
		_data[end] = '\0';
		_entries.push_back(&_data.front() + end + 1);
	}

	_entries.pop_back(); // Last line is always null terminated
}
