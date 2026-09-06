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

#ifndef __PVCOP_TYPES_COLLECTOR_H__
#define __PVCOP_TYPES_COLLECTOR_H__

#include <pvcop/core/memarray.h>

#include <pvcop/db/collector.h>

#include <pvcop/formatter_desc_list.h>

#include <vector>
#include <memory>

namespace pvcop
{

class write_dict;

namespace types
{

class formatter_interface;

} // namespace pvcop::types

class sink;
class sel_write_handler;

/**
 * This class handles database creation using formatters.
 */
class collector : public db::collector
{
	friend class sink;

	using formatters_t = std::vector<std::unique_ptr<types::formatter_interface>>;

  public:
	/**
	 * Constructor
	 *
	 * @param rootdir the collector root directory (see @pvcop::db::collector)
	 * @param formatter_descs the formatter descriptor list (see pvcop::formatter_desc_list)
	 *
	 * @throw pvcop::types::exception::unknown_formatter_error
	 */
	collector(const char* rootdir, const formatter_desc_list& formatter_descs);
	~collector(); // destructor is needed to use std::unique_ptr with incomplete type

  public:
	/**
	 * @overrides db::collector::close
	 *
	 * @throw std::ios::failure in case of error
	 */
	void close();

  protected:
	/**
	 * Gets the formatter of a column
	 *
	 * @param index the wanted column
	 *
	 * @return a pointer of a formatter_interface
	 */
	const types::formatter_interface* formatter(size_t index) const
	{
		return _formatters[index].get();
	}

  private:
	using column_invalid_dicts = std::vector<std::unique_ptr<pvcop::write_dict>>;

  private:
	column_invalid_dicts _invalid_dicts;

	formatter_desc_list _formatter_descs;
	formatters_t _formatters;
};

} // namespace pvcop

#endif // __PVCOP_TYPES_COLLECTOR_H__
