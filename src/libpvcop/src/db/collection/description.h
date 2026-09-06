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

#ifndef __PVCOP_DB_DESCRIPTION_H__
#define __PVCOP_DB_DESCRIPTION_H__

#include <pvcop/db/types.h>

#include <string>
#include <functional>

namespace pvcop
{

namespace db
{

/**
 * This class provides loading and saving methods from/to the description file
 * of a pvcop data directory.
 *
 * A description file is an intelligible ASCII text file starting with its
 * format version and followed by the data set informations.
 *
 * The version 1 of the file has a structure which is given by the following
 * EBNF notation (including the format version):
 *
 * @code
 * description = header column-information-list
 *
 * header = "1" "\n" row-number "\n" column-number "\n"
 *
 * column-information-list = { identifier " " type-name "\n" }
 *
 * row-number = NUMBER
 *
 * column-number = NUMBER
 *
 * identifier = NUMBER
 *
 * type-name = DB_TYPE_NAME (* see type_t for further informations *)
 * @endcode
 *
 * @todo write about the effect of add/remove/whatever on a collection and
 * why the field 'id' is required (teasing: a file_read_handler can
 * be null:)
 */
class description
{
  public:
	/**
	 * the extension to use for data file names
	 */
	static constexpr const char* data_file_extension = ".data";

	/**
	 * the extension to use for dictionary file names
	 */
	static constexpr const char* dict_file_extension = ".dict";

	/**
	 * the extension to use for invalid selection file names
	 */
	static constexpr const char* invalid_sel_file_extension = ".isel";

	/**
	 * the extension to use for invalid dictionary file names
	 */
	static constexpr const char* invalid_dict_file_extension = ".idict";

  public:
	/**
	 * Function called after the header has been read
	 *
	 * Parameters are:
	 * -# the stored row number
	 * -# the stored column number
	 */
	using process_header_f = std::function<void(size_t, size_t)>;

	/**
	 * function called for each column information read
	 *
	 * Parameters are:
	 * -# the data file name
	 * -# the dictionary file name
	 * -# the stored column identifier
	 * -# the stored column type
	 */
	using process_column_f =
	    std::function<void(const std::string&, const std::string&, size_t, type_t)>;

	/**
	 * Function to get the information about a column.
	 *
	 * Parameters are:
	 * -# the index of the wanted column
	 * Return :
	 * -# a reference to store the column type to
	 *
	 * Returns true if a column exists for this index; false otherwise.
	 */
	using get_column_info_f = std::function<type_t(size_t)>;

  public:
	/**
	 * Load the informations from a description file to initialize an
	 * overlaying structure.
	 *
	 * @param rootdir the data root directory
	 * @param process_header a function called with header information
	 * @param process_column a function called for each read column information
	 *
	 * @throw std::ios::failure in case of error
	 */
	static void load(const std::string& rootdir,
	                 const process_header_f& process_header,
	                 const process_column_f& process_column);

	/**
	 * Save to the description file informations from an overlaying structure.
	 *
	 * @param rootdir the data root directory
	 * @param row_count the data actual row number
	 * @param row_count the data actual column number
	 * @param get a function to retrieve informations for a given column
	 *
	 * @throw std::ios::failure in case of error
	 */
	static void save(const std::string& rootdir,
	                 const size_t row_count,
	                 const size_t column_count,
	                 const get_column_info_f& get);

	static void append_column(const std::string& rootdir,
	                          const size_t column_count,
							  const get_column_info_f& get);

	static void remove_column(const std::string& rootdir, size_t column_index);
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_DESCRIPTION_H__
