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

#ifndef __PVCOP_DB_SINK_H__
#define __PVCOP_DB_SINK_H__

#include <pvcop/core/mempage.h>
#include <pvcop/core/pagedarray.h>

#include <pvcop/db/types.h>

#include <pvcop/types/formatter/formatter_interface.h>

#include <cstddef>
#include <vector>

namespace pvcop
{

namespace db
{

class collector;
class write_dict;

/**
 * This class is an accessor to write row oriented data in a collector (which
 * store data as column oriented)
 *
 * @todo write a full explanation
 */
class sink
{
  public:
	struct column_chunk_t {
		column_chunk_t() = default;
		column_chunk_t(const void* b, size_t rc, size_t rbc, pvcop::db::type_t t)
		    : buffer(b), row_count(rc), row_bytes_count(rbc), type(t)
		{
		}

		const void* buffer = nullptr;
		size_t row_count = 0;
		size_t row_bytes_count = 0;
		pvcop::db::type_t type;
	};
	using columns_chunk_t = std::vector<column_chunk_t>;

  public:
	/**
	 * Default constructor
	 *
	 * @param c a reference to the target collector
	 */
	explicit sink(collector& c);

	/**
	 * Destructor
	 */
	~sink();

  public:
	void write_chunk_by_column(const size_t begin_row,
	                           const size_t row_count,
	                           const sink::columns_chunk_t& columns);

	void set_column_dict(size_t column_index, std::unique_ptr<db::write_dict> dict);

	void set_chunk_null_bitmap(size_t col, size_t start_row, size_t row_count, const uint8_t* chunk_null_bitmap);


  protected:
	/**
	 * Gets the type of a column
	 *
	 * @param index the wanted column index
	 *
	 * @return the type of the wanted column
	 *
	 * @throw pvcop::db::exception::invalid_column if @p index is out of range
	 */
	type_t type(size_t index) const;

	/**
	 * Gets the write dictionary associated with the column @a col.
	 *
	 * @param col the wanted column index
	 *
	 * @return a pointer on the associated write dictionary
	 *
	 * @throw pvcop::db::exception::invalid_column if @p index is out of range
	 */
	write_dict* dict(size_t index) const;

  protected:
	/**
	 * Gets the mempage of a given column
	 *
	 * @param col the wanted column index
	 *
	 * @return a reference on a mempage
	 */
	core::mempage& page(size_t col)
	{
		assert(_pages.size() > col);
		return _pages[col];
	}

	/**
	 * Gets the mempage of a given column (const version)
	 *
	 * @param col the wanted column index
	 *
	 * @return a reference on a mempage
	 */
	const core::mempage& page(size_t col) const
	{
		assert(_pages.size() > col);
		return _pages[col];
	}

  protected:
	/**
	 * Value accessor for a given column-row pair
	 *
	 * @param col the wanted column index
	 * @param row the global wanter row index
	 *
	 * @return a typed accessor on a value in the collector
	 */
	template <typename T>
	typename core::array<T>::accessor at(size_t col, size_t row)
	{
		core::mempage& p = page(col);
		core::pagedarray<T> a(p);
		return a[p.relative_index(row)];
	}

  protected:
	/**
	 * Request a mempage for a given column
	 *
	 * @param col the column index
	 * @param row the row index
	 *
	 * @param return true on success; false otherwise
	 */
	bool request_page(const size_t col, const size_t row);

	/**
	 * Release a mempage for a given column
	 *
	 * @param col the column index
	 */
	void release_page(const size_t col);

	/**
	 * Checks for page fault for a given column (see pvcop::core::mempage::check_fault)
	 *
	 * @param col the column index to check
	 * @param row the row index to check
	 *
	 * @return 0 if index does not match the page; the remaining number of
	 * element places otherwise
	 */
	size_t check_fault(const size_t col, const size_t row) const;

	/**
	 * Checks for page fault for all mempages (see pvcop::core::mempage::check_fault)
	 *
	 * @param row the row index to check
	 *
	 * @return 0 if one page is faulty; the number of element places before
	 * the next fault otherwise
	 */
	size_t check_fault(const size_t row) const;

	/**
	 * Makes sure all pages are valid for a given row index
	 *
	 * @param row the wanted row index
	 *
	 * @return @c true if all pages are valid; @c false otherwise
	 */
	bool refresh_pages(const size_t row);
	bool refresh_page(const size_t col, const size_t row);

  protected:
	/**
	 * Increments the target collector row count
	 *
	 * @param count the value to add to the row count
	 *
	 * @note this method is thread-safe
	 */
	void increment_row_count(const size_t count);

  protected:
	using pages_t = std::vector<core::mempage>;
	pages_t _invalid_sel_pages;

  private:
	collector& _collector;
	pages_t _pages;
	pvcop::types::formatter_interface::shared_ptr _string_formatter;
};

} // namespace db

} // namespace pvcop

#endif // __PVCOP_DB_SINK_H__
