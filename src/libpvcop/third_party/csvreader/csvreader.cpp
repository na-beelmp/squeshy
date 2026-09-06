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

#include "csvreader.h"

#include <cstring>

#include <pvlogger.h>

/*****************************************************************************
 * csv::reader::reader
 *****************************************************************************/

csv::reader::reader(char* buffer,
                    const size_t buffer_size,
                    const size_t column_count,
                    char vsep,
                    char sdel,
                    bool strict_mode)
    : _buffer(buffer)
    , _buffer_size(buffer_size)
    , _column_count(column_count)
    , _nread(0)
    , _vsep(vsep)
    , _sdel(sdel)
    , _strict_mode(strict_mode)
    , _ptr(buffer)
{
}

/*****************************************************************************
 * csv::reader::~reader
 *****************************************************************************/

csv::reader::~reader()
{
}

/*****************************************************************************
 * csv::reader::read_next_line
 *****************************************************************************/

char* csv::reader::read_next_line(size_t* buffer_line_size)
{
	char* old_ptr = _ptr;
	_ptr = (char*)memchr(_ptr, '\n', (_buffer + _buffer_size - _ptr));

	if (buffer_line_size != nullptr) {
		*buffer_line_size = _ptr - old_ptr;
	}

	_ptr++;

	return old_ptr;
}

/*****************************************************************************
 * csv::reader::process_line
 *****************************************************************************/

bool csv::reader::process_line(entry_info_t* entries,
                               const char* buffer,
                               const size_t buffer_len) const
{
	size_t i = 0;
	size_t pos = 0;
	size_t len = 0;
	size_t index = 0;

	while (i < buffer_len) {
		if (index == _column_count) {
			return false;
		}

		if (!_strict_mode && (buffer[i] == _vsep)) {
			while ((i < buffer_len) && (buffer[i] == _vsep)) {
				++i;
				++pos;
			}
		}

		if (buffer[i] == _sdel) {
			// a "multi-words" value
			++i;
			++pos;
			while ((i < buffer_len) && (buffer[i] != _sdel)) {
				++i;
				++len;
			}
			if (i == buffer_len) {
				// EOL but not a value delimiter
				return false;
			}
			++i;
			if (i == buffer_len) {
				// EOL
				break;
			}
			if (buffer[i] == _vsep) {
				entries[index].pos = pos;
				entries[index].len = len;
				++index;
				++i;
				pos = i;
				len = 0;
			} else {
				// not EOL but not a value sev
				return false;
			}
		} else {
			// a "one-word" field
			while ((i < buffer_len) && (buffer[i] != _vsep)) {
				++i;
				++len;
			}
			if (i == buffer_len) {
				// EOL
				break;
			}
			entries[index].pos = pos;
			entries[index].len = len;
			++index;
			++i;
			pos = i;
			len = 0;
		}
	}
	entries[index].pos = pos;
	entries[index].len = len;
	++index;

	return index == _column_count;
}
