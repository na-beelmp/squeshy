/* * MIT License
 *
 * © ESI Group, 2015
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CSV_READER_H
#define CSV_READER_H

#include <cstddef>
#include <cstdio>

namespace csv
{

class reader
{
  public:
	struct entry_info_t {
		size_t pos;
		size_t len;
	};

  public:
	reader(char* buffer,
	       const size_t buffer_size,
	       const size_t column_count,
	       char vsep = ',',
	       char sdel = '"',
	       bool strict_mode = true);

	~reader();

	void set_strict_mode(bool strict_mode) { _strict_mode = strict_mode; }
	bool get_strict_mode() { return _strict_mode; }

	const char* buffer() { return _buffer; }
	size_t buffer_len() { return _nread; }

	int get_column_count() { return _column_count; }

	char* read_next_line(size_t* buffer_line_size);
	bool process_line(entry_info_t* entries, const char* buffer, const size_t buffer_len) const;

  private:
	const char* _buffer;
	size_t _buffer_size;
	size_t _column_count;
	size_t _nread;
	char _vsep;
	char _sdel;
	bool _strict_mode;

	char* _ptr;
};

} // namespace csv

#endif // CSV_READER_H
