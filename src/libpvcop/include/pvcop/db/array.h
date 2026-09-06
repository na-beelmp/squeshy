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

#ifndef __PVCOP_DB_ARRAY_H__
#define __PVCOP_DB_ARRAY_H__

#include <pvcop/db/array_base.h>
#include <pvcop/types/formatter/formatter_interface.h>

#include <string>
#include "exceptions/size_mismatch_error.h"

namespace pvcop
{

namespace db
{

/**
 * A high level read-only fixed-size sequence container offering a set of database primitive
 * methods.
 */
class array : public array_base
{
  public: // TODO: better visibility
	      /**
	       * invalid constructor
	       */
	array();

	/**
	 * initialization constructor
	 *
	 * @param type the type of the array as a string
	 * @param count the number of elements contained in the array
	 *
	 * @throw pvcop::db::exception::no_string_array_error
	 */
	explicit array(type_t type, size_t count = 0, bool init = false);

	/**
	 * slice constructor
	 *
	 * @param pimpl the original array pimpl
	 * @param pos the start position in @a array
	 * @param len the size of the new array
	 */
	array(const array_impl_interface* pimpl, size_t pos, size_t len);

	/**
	 * pimpl constructor
	 *
	 * @param pimpl the pimpl created by the array_factory
	 */
	explicit array(array_impl_interface* pimpl);

	/**
	 * No copy constructor
	 */
	array(const db::array&) = delete;

	/**
	 * default move constructor
	 */
	array(db::array&&) = default;

	/**
	 * default move assignment operator
	 */
	array& operator=(array&&) = default;

	/**
	 * Prevent such use of the comparison operator,
	 * because db::array(0) resolve to db::array(array_impl_interface*) constructor.
	 *
	 * db::array a(type_int32, 1);
	 * if (a == 0 || a == 0UL) {
	 *     // this should not compile !
	 * }
	 */
	array(unsigned int t) = delete;

  public:
	/**
	 * destructor
	 */
	~array();

  public:
	/**
	 * return a slice from an array
	 *
	 * @param pos the starting index
	 * @param len the length of the final slice
	 *
	 * @return an array on [pos,pos+len[
	 *
	 * @note no memory allocation occurs during slicing
	 */
	db::array slice(size_t pos, size_t len) const;

	/**
	 * merge two arrays together.
	 *
	 * @param array2 the array to append.
	 *
	 * @return an array composed of the concatenation of the two arrays.
	 */
	db::array concat(const db::array& array2) const;
	static db::array concat(const std::vector<db::array>& arrays);

	/**
	 * @return a copy of an array
	 */
	db::array copy() const;

  public:
	/**
	 * Return the underlying formatter
	 */
	pvcop::types::formatter_interface::shared_ptr formatter() const;

	/**
	 * Specify how the underlying values are converted to strings.
	 * The formatter is needed by the "at" methods.
	 *
	 * @param formatter the formatter to use.
	 */
	void set_formatter(const pvcop::types::formatter_interface::shared_ptr formatter);

  public:
	/**
	 * returns a string representation of a specific row
	 *
	 * @param row the specified row index
	 * @param str the buffer to write to
	 * @param str_len the length of the buffer
	 *
	 * @note a formatter should have been set using "set_formatter"
	 *
	 * @return the written size or a negative value to report an error
	 */
	int at(size_t row, char* str, const size_t str_len) const;

	/**
	 * helper method to return a std::string representation of a specific row
	 *
	 * @param row the specified row index
	 * @param str_len the maximum supported length
	 *
	 * @return the string representation of the specified row
	 *         or a null string if an error occured
	 *
	 * @note a formatter should have been set using "set_formatter"
	 * @note a buffer copy occurs using this method.
	 */
	std::string at(size_t row,
	               size_t str_len = types::formatter_interface::MAX_STRING_LENGTH) const;


	/**
	 * returns the length of the longest string stored in the dictionnary
	 *
	 * @return the longest length or 0 if array is not of type "string"
	 */
	size_t max_string_length() const;

	/**
	 * returns the dictionary of distinct strings
	 *
	 * @return the dict or nullptr if array is not of type "string"
	 */
	const read_dict* dict() const;
};

// Defining strengthened db::array types:

class indexes : public array_base
{
  public:
	using type = pvcop::db::index_t;

  public:
	indexes();
	explicit indexes(size_t count);
	indexes(db::indexes&&) = default;
	indexes& operator=(indexes&& rhs);
	~indexes();

  protected:
	friend class array_base;
	explicit indexes(array_impl_interface* pimpl);

  public:
	db::indexes slice(size_t pos, size_t len) const;
	db::indexes concat(const indexes& b) const;

  public:
	/**
	 * As the underlying type is known, no need to specify it
	 * as a template parameter unlike db::array::to_core_array
	 *
	 * @note : Provide const& and & overload to remove && overload and
	 * have a compile time error for : pvcop::db::algo::my_algo(input_data).to_core_array<...>();
	 * as result is a dangling reference.
	 */
	const core::array<type>& to_core_array() const&;
	core::array<type>& to_core_array() &;
	core::array<type>& to_core_array() && = delete;
};

/**
 * Index list mapping line_id to a group_id from an extents.
 */
class groups : public indexes
{
  public:
	explicit groups(size_t count);
	groups();
	groups(db::groups&&) = default;
	groups(unsigned int t) = delete;
	~groups();

  public:
	db::groups slice(size_t pos, size_t len) const;
	db::groups concat(const groups& b) const;

  protected:
	friend class array_base;
	explicit groups(array_impl_interface* pimpl);
};

/**
 * Index list to the first occurrence of a given value
 */
class extents : public indexes
{
  public:
	explicit extents(size_t count);
	extents();
	extents(db::extents&&) = default;
	extents(unsigned int t) = delete;
	~extents();

  public:
	db::extents slice(size_t pos, size_t len) const;
	db::extents concat(const extents& b) const;

  protected:
	friend class array_base;
	explicit extents(array_impl_interface* pimpl);
};

/**
 * Build a standalone string array from a list of values.
 *
 * A string column stores indices into a dictionary, and that dictionary
 * normally belongs to the db::collection the column was read from -- so an
 * array of type "string" built with the plain constructor has none, and reading
 * a value from it dereferences a null dictionary. This builds both: the
 * distinct values become the dictionary, which the array's formatter owns, so
 * the array is self-contained and can be handed to anything expecting a string
 * column without a collection behind it.
 *
 * Values repeat as often as they occur in @a values; the dictionary holds each
 * one once, in first-seen order.
 *
 * @param values the strings the array holds, in order
 *
 * @return an array of type "string" of the same size as @a values
 */
array make_string_array(const std::vector<std::string>& values);

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_ARRAY_H__
