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

#ifndef __PVCOP_DB_COLLECTION_H__
#define __PVCOP_DB_COLLECTION_H__

#include <pvcop/db/array.h>
#include <pvcop/db/types.h>

#include <vector>
#include <string>
#include <memory>

namespace pybind11
{
class array;
}

namespace pvcop
{

namespace db
{

class file_read_handler;
class read_dict;

/**
 * This class centralizes the management of structured data stored on disk. It
 * permits to retrieve the data of a column as a db::array.
 *
 * It uses lazy mapping/unmapping to limit memory usage and it takes advantage
 * of Linux disk-cache laziness to handle efficently later remappings.
 *
 * It only permits to entirely map a file; it is also not possible to do
 * partial mapping.
 *
 * @warning even though any db::array retrieved from a db::collection remains
 * valid after its db::collection destruction, the good practice is to close a
 * db::collection after the destruction of all of its related db::array.
 */
class collection
{
  public:
	/**
	 * Constructor
	 *
	 * @param rootdir the path to the root directory used to store data
	 *
	 * @warning opening a collection more than one time may have undefined behavior
	 */
	explicit collection(std::string const& rootdir);

	/**
	 * No copy constructor
	 */
	collection(collection&) = delete;

	/**
	 * No move constructor
	 */
	collection(collection&&) = delete;

	/**
	 * Destructor
	 */
	~collection();

  public:
	/**
	 * No copy assignment operator
	 */
	collection& operator=(collection&) = delete;

	/**
	 * No move assignment operator
	 */
	collection& operator=(collection&&) = delete;

  public:
	/**
	 * Convert to bool to check validity
	 *
	 * @return @c true if the collection is open and valid; @c false otherwise
	 */
	operator bool() const;

	/**
	 * Retrieve the rows count of the collection
	 *
	 * @return the number of row
	 */
	size_t row_count() const { return _row_count; }

	/**
	 * Retrieve the columns count of the collection
	 *
	 * @return the number of column
	 */
	size_t column_count() const { return _column_count; }

	/**
	 * Gets root directory
	 *
	 * @return the collection root directory
	 */
	const std::string& rootdir() const { return _rootdir; }

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
	array column(size_t index) const;

	/**
	 * Gets the read dictionary associated with the column @a col.
	 *
	 * @param index the wanted column index
	 *
	 * @return a pointer on the associated read dictionary
	 *
	 * @throw pvcop::db::exception::invalid_column if @p index is out of range
	 */
	const read_dict* dict(size_t index) const;

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
	 * Append a new column to the collection
	 *
	 * @param column_type the type of the column
	 * @param column pybind11 numpy array
	 *
	 * @return true if success, false otherwise
	 */
	bool append_column(const pvcop::db::type_t& column_type, const pybind11::array& column);

	/**
	 * Delete a column from the collection
	 *
	 * @param column_index the index of the column
	 */
	void delete_column(size_t column_index);

  private:
	/**
	 * deduplicate_strings
	 *
	 * @param column pybind11 numpy fixed string array
	 * @param write_dict the resulting string dictionnary
	 *
	 * @return the created values referencing strings in the dictionnary
	 */
	std::vector<db::write_dict::index_type> deduplicate_strings(const pybind11::array& column, db::write_dict& write_dict);

    /**
	 * Save the collection description to disk
	 *
	 * @throw pvcop::db::exception::invalid_column if some handlers are uninitialized
	 */
    void save_description();

    /**
	 * Remove a column from the description file
	 *
	 * @param column_index the index of the column to remove
	 */
	void remove_column_from_description(size_t column_index);

  public:
	/**
	 * Close the collection for any new access
	 *
	 * @throw std::ios::failure in case of error
	 *
	 * @warning : Every column loaded from collection have to be destroy before the collection
	 *close. Otherwise random memory read happen.
	 */
	void close();

  private:
	using column_handlers = std::vector<std::unique_ptr<file_read_handler>>;
	using column_dicts_t = std::vector<std::unique_ptr<read_dict>>;

  protected:
	std::string _rootdir;
	size_t _row_count;
	size_t _column_count;
	column_handlers _handlers;
	column_dicts_t _dicts;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_COLLECTION_H__
