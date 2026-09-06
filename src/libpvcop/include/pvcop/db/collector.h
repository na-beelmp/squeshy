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

#ifndef __PVCOP_DB_COLLECTOR_H__
#define __PVCOP_DB_COLLECTOR_H__

#include <atomic>
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include <pvcop/db/format.h>
#include <pvcop/db/sel_write_handler.h>

namespace pvcop
{

namespace db
{

class file_write_handler;
class write_dict;

/**
 * This class abstracts  which centralizes a database creation.
 *
 * @todo is associating a collector with a collection meaningful to permit
 * using a collection while appending new records?
 */
class collector
{
	friend class sink;

  public:
	using row_counter = std::atomic_size_t;

  public:
	/**
	 * Default constructor
	 *
	 * @param rootdir the path to the root directory used to store data
	 * @param f the format of the data
	 *
	 * @throw pvcop::db::exception::collector_error in case of error
	 */
	collector(const char* rootdir, const format& f);

	/**
	 * Destructor
	 */
	~collector();

  public:
	/**
	 * Closes the collector for any new insertion
	 *
	 * @throw std::ios::failure in case of error
	 */
	void close();

  public:
	/**
	 * Convert to bool to check collector validity
	 *
	 * @return @c true if the collector is valid; @c false otherwise
	 */
	operator bool() const;

	/**
	 * Get the current row count
	 *
	 * @return the current row count
	 */
	size_t row_count() const { return _row_count; }

	/**
	 * @return the sanitized root directory
	 */
	std::string rootdir() const { return _rootdir; }

	/**
	 * Get the collector column count
	 *
	 * @return the collector column count
	 */
	size_t column_count() const { return _column_count; }

  protected:
	/**
	 * Retrieve the type of a column given its index
	 *
	 * @param index the wanted column index
	 *
	 * @return the type of the index-th column
	 *
	 * @throw pvcop::db::exception::invalid_column if @p index is out of range
	 */
	type_t type(size_t index) const;

	/**
	 * Gets the write dictionary associated with the column @a col.
	 *
	 * @param index the wanted column index
	 *
	 * @return a pointer on the associated write dictionary
	 *
	 * @throw pvcop::db::exception::invalid_column if @p index is out of range
	 */
	write_dict* dict(size_t index) const;

	/**
	 * Set the write dictionary associated with the column @a col.
	 *
	 * @param index the wanted column index
	 * 
	 * @param dict unique_ptr on the associated write dictionary
	 *
	 * @throw pvcop::db::exception::invalid_column if @p index is out of range
	 */
	void set_dict(size_t index, std::unique_ptr<db::write_dict> dict);

	/**
	 * Get a reference on the collector column counter
	 *
	 * @return a reference on the collector column counter
	 */
	row_counter& get_row_counter() { return _row_count; }

  protected:
	/**
	 * Frees all existing handler
	 */
	void clean_handlers();

	/**
	 * Frees all existing dictionary
	 */
	void clean_dicts();

  protected:
    using column_invalid_handlers = std::vector<std::unique_ptr<sel_write_handler>>;

  protected:
	column_invalid_handlers _invalid_sel_handlers;
	/* Not a vector<bool>: sinks flag columns concurrently, and the bit-packed
	 * specialisation would turn '_invalid_columns[col] = true' into a read-modify-write
	 * of the word holding 64 columns, letting two threads lose each other's writes. */
	std::vector<uint8_t> _invalid_columns;

  private:
	using column_handlers = std::vector<std::unique_ptr<file_write_handler>>;
	using column_dicts = std::vector<std::unique_ptr<write_dict>>;

  protected:
	std::string _rootdir;
	row_counter _row_count;
	size_t _column_count;

  private:
	column_handlers _handlers;
	column_dicts _dicts;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_COLLECTOR_H__
