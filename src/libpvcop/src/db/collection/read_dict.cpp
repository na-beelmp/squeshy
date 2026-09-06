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

#include <algorithm>
#include <numeric>
#include <fstream>
#include <filesystem>

/*****************************************************************************
 * pvcop::db::read_dict::read_dict
 *****************************************************************************/

pvcop::db::read_dict::read_dict()
= default;

/*****************************************************************************
 * pvcop::db::read_dict::read_dict
 *****************************************************************************/

pvcop::db::read_dict::read_dict(const std::string& filename)
{
	load(filename);
}

/*****************************************************************************
 * pvcop::db::read_dict::read_dict
 *****************************************************************************/

pvcop::db::read_dict::read_dict(const string_set_t& strings)
{
	create(strings);
}

/*****************************************************************************
 * pvcop::db::read_dict::load
 *****************************************************************************/

void pvcop::db::read_dict::load(const std::string& filename)
{
	/* open the content file in binary+"seek at end" modes to retrieve its real size as C++11
	 * does not provide any filesystem oriented classes/functions to do it.
	 */
	std::ifstream file{std::filesystem::path(filename), std::ios::binary | std::ios::ate};

	if (!file.is_open()) {
		throw std::ios::failure(std::string("can not open file '" + filename + "' for reading"));
	}

	std::streamsize file_size = file.tellg();

	file.seekg(0, std::ios::beg);

	/* do a read at once to avoid reallocation and muliple read system calls
	 */
	_data.resize(file_size + 1);
	file.read(&_data[0], file_size);

	/* make the last string a valid C string
	 */
	_data[file_size] = '\0';

	finalize();
}

/*****************************************************************************
 * pvcop::db::read_dict::create
 *****************************************************************************/

void pvcop::db::read_dict::create(const string_set_t& strings)
{
	assert(strings.size() != 0);

	/* first, we compute the storage space required to load all strings including their terminal
	 * new-line characters.
	 */
	size_t data_size = std::accumulate(
	    strings.begin(), strings.end(), 0,
	    [](const size_t size, const std::string& str) -> size_t { return size + str.size() + 1; });

	_data.reserve(data_size);

	/* copy time
	 */
	size_t count = strings.size() - 1;

	for (size_t i = 0; i < count; ++i) {
		_data += strings[i];
		_data += '\n';
	}

	_data += strings[count];

	finalize();
}

/*****************************************************************************
 * pvcop::db::read_dict::finalize
 *****************************************************************************/

void pvcop::db::read_dict::finalize()
{
	/* build words set
	 */
	size_t word_count = std::count(_data.begin(), _data.end(), '\n') + 1;

	_words.reserve(word_count);

	size_t last_i = 0;

	// the N-1 first words
	for (size_t i = 0; i < _data.size(); ++i) {
		if (_data[i] == '\n') {
			_data[i] = '\0';
			_words.push_back(_data.data() + last_i);
			last_i = i + 1;
		}
	}

	// and the last one
	_words.push_back(_data.data() + last_i);

	/* time to fill the index
	 */
	_index.clear();

	// set an optimal size
	_index.rehash(word_count);

	for (const auto& word : _words) {
		_index.insert(word);
	}
}
