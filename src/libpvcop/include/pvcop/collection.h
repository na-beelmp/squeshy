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

#ifndef __PVCOP_COLLECTION_H__
#define __PVCOP_COLLECTION_H__

#include <pvcop/formatter_desc_list.h>
#include <pvcop/db/collection.h>
#include <pvcop/db/array.h>

#include <memory>

namespace pybind11
{
class array;
}

namespace pvcop
{

class sel_read_handler;

namespace types
{

class formatter_interface;

} // namespace pvcop::types

/**
 * This class handles use of database with formatters.
 */
class collection : public db::collection
{
  public:
	using formatter_sp = std::shared_ptr<types::formatter_interface>;
	using formatters_t = std::vector<formatter_sp>;

  public:
	/**
	 * Constructor
	 *
	 * @param rootdir the collection root directory
	 *
	 * @throw pvcop::types::exception::unknown_formatter_error
	 */
	explicit collection(std::string const& rootdir);
	~collection(); // destructor is needed to use std::unique_ptr with incomplete type

  public:
	/**
	 * Retrieve a db::array given its column index
	 *
	 * @param index the wanted column index
	 *
	 * @return a db::array for the index-th column; if an error occurs, a
	 * null db::array is returned.
	 *
	 * @note errors can be due to an out-of-range index, an inability
	 * to give an access to stored data, or a memory allocation failure.
	 */
	pvcop::db::array column(size_t index) const;


	bool append_column(const pvcop::db::type_t& column_type, const pybind11::array& column);

	void delete_column(size_t column_index);

	/**
	 * Convert to bool to check validity
	 *
	 * @return @c true if the collection and format are valid; @c false otherwise
	 */
	operator bool() const;

  private:
	formatters_t _formatters;
	formatter_desc_list _formatter_descs;

	using column_invalid_handlers = std::vector<std::unique_ptr<sel_read_handler>>;
	using column_invalid_dicts = std::vector<std::unique_ptr<db::read_dict>>;

	column_invalid_handlers _invalid_sel_handlers;
	column_invalid_dicts _invalid_dicts;
};
} // namespace pvcop

#endif // __PVCOP_COLLECTION_H__
