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

#ifndef PVCOP_TYPES_FORMATTER_STRING_H
#define PVCOP_TYPES_FORMATTER_STRING_H

#include <pvcop/types/formatter/formatter.h>
#include <pvcop/db/types.h>
#include <pvcop/db/read_dict.h>
#include <pvcop/db/write_dict.h>

#include <cstring>

namespace pvcop
{

class collector;
class collection;

namespace types
{

class formatter_string : public formatter<string_index_t>
{
	friend class pvcop::collector;
	friend class pvcop::collection;

  public:
	/**
	 * Constructor
	 */
	explicit formatter_string(const char* p)
	    : formatter<type>(p), _r_dict(nullptr), _w_dict(nullptr)
	{
	}

  public:
	/**
	 * override pvcop::types::formatter_interface::name
	 */
	std::string name() const final { return "string"; }

	/**
	 * override pvcop::types::formatter_interface::parameter
	 */
	const char* parameters() const final { return nullptr; }

	/**
	 * override pvcop::types::formatter_interface::set_parameters
	 */
	void set_parameters(const char*) override {}

  protected:
	/**
	 * Convert a string to its internal value.
	 *
	 * It insert data in dictionnary if we are in write mode.
	 * It only search maching value in read mode.
	 */
	bool convert_from_string(const char* str, reference value, bool* pass_autodetect) const override
	{
		if (pass_autodetect and str[0] == '\0') {
			*pass_autodetect = false;
		}

		if (_r_dict) {
			value = _r_dict->index(str);
			return value != db::read_dict::invalid_index;
		} else {
			value = _w_dict->insert(str);
			return value != db::write_dict::invalid_index;
		}
	}

	/**
	 * Convert an internal value to its string representation.
	 *
	 * Only available in read mode.
	 */
	int convert_to_string(char* str, const size_t str_len, const type& value) const override
	{
		const char* src = _r_dict->key(value);

		if (src == nullptr) {
			// data[i] is out of range
			return -1;
		}

		return std::snprintf(str, str_len, "%s", src);
	}

  protected:
	/**
	 * Sets the db::read_dict to use
	 *
	 * @param dict the read_dict
	 *
	 * @note it implictly invalidates the write dictionary.
	 */
	void set_read_dict(const db::read_dict* dict)
	{
		_r_dict = dict;
		_w_dict = nullptr;
	}

	/**
	 * Sets the db::write_dict to use
	 *
	 * @param dict the write_dict
	 *
	 * @note it implictly invalidates the read dictionary.
	 */
	void set_write_dict(db::write_dict* dict)
	{
		_w_dict = dict;
		_r_dict = nullptr;
	}

  public:
	/**
	 * Sets a db::read_dict this formatter takes ownership of.
	 *
	 * A string column normally borrows the dictionary held by its
	 * db::collection, which is why set_read_dict() takes a raw pointer and is
	 * reserved to the collector and the collection. An array built in memory,
	 * outside of any collection, has nowhere to keep one -- so it keeps it
	 * here, and the dictionary lives as long as the formatter, hence as long as
	 * the array sharing it.
	 *
	 * @param dict the read_dict to own
	 *
	 * @note it implicitly invalidates the write dictionary.
	 */
	void set_owned_read_dict(std::unique_ptr<db::read_dict> dict)
	{
		_owned_dict = std::move(dict);
		_r_dict = _owned_dict.get();
		_w_dict = nullptr;
	}

  private:
	const db::read_dict* _r_dict; //!< Dictionnary to reading value.
	db::write_dict* _w_dict;      //!< Dictionnary to write value.
	//! Set only when this formatter owns its dictionary; _r_dict then aliases it.
	std::unique_ptr<db::read_dict> _owned_dict;
};

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_FORMATTER_STRING_H
