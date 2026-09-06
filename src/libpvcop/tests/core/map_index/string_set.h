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

#ifndef __PVCOP_TEST_CORE_CUI_STRING_SET_H__
#define __PVCOP_TEST_CORE_CUI_STRING_SET_H__

#include <vector>
#include <string>

/**
 * Extract data from a file and save cstring ptr for each line (keep a single buffer)
 */
class string_set
{
  public:
	using entries_t = std::vector<const char*>;
	using data_t = std::string;

  public:
	string_set(std::string const& filename);

  public:
	const entries_t& entries() const { return _entries; }

	const data_t& data() const { return _data; }

  private:
	entries_t _entries;
	data_t _data;
};

#endif // __PVCOP_TEST_CORE_CUI_STRING_SET_H__
