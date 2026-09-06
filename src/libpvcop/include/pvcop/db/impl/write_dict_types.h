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

#ifndef __PVCOP_DB_IMPL_WRITE_DICT_TYPES_H__
#define __PVCOP_DB_IMPL_WRITE_DICT_TYPES_H__

#include <pvcop/db/impl/cstring_traits.h>

namespace pvcop
{

namespace db
{

namespace __impl
{

/**
 * This class is used as key type for a core::map_index to implement
 * the string type support through the class pvcop::db::write_dict.
 *
 * To avoid duplicating the string each time @b map_index::insert() is
 * called, we want to have the allocation only when the (internal) insertion is
 * done.
 */
class write_dict_string_t
{
  public:
	/**
	 * Constructor by parameter
	 */
	write_dict_string_t(const char* v) : _guard_value(v), _value(const_cast<char*>(v)) {}

	/**
	 * copy constructor
	 *
	 * It does the effective copy
	 */
	write_dict_string_t(const write_dict_string_t& rhs) : _guard_value(rhs._guard_value)
	{
		size_t s = strlen(rhs._value) + 1;
		_value = new char[s];
		strncpy(_value, rhs._value, s);
	}

	write_dict_string_t& operator=(write_dict_string_t const&) = delete;
	write_dict_string_t& operator=(write_dict_string_t&&) = delete;
	write_dict_string_t(write_dict_string_t&& rhs) = delete;

	/**
	 * Destructor
	 */
	~write_dict_string_t()
	{
		if (_value != _guard_value) {
			delete[] _value;
		}
	}

  public:
	/**
	 * Get the key value
	 *
	 * @return the key value
	 */
	const char* value() const { return _value; }

  private:
	const char* _guard_value;
	char* _value;
};

/**
 * Structure conform to tbb::tbb_hash_compare<T> to use with the internal
 * container of class pvcop::core::map_index.
 */
struct write_dict_string_hashcompare_t {
	/**
	 * Constructor
	 */
	write_dict_string_hashcompare_t() {}

	/**
	 * Copy constructor
	 */
	write_dict_string_hashcompare_t(const write_dict_string_hashcompare_t&) {}

	/**
	 * Destructor
	 */
	~write_dict_string_hashcompare_t() {}

	/**
	 * Checks for the equality of 2 keys.
	 *
	 * @param lhs the first key
	 * @param rhs the other key
	 *
	 * @return true if the 2 keys are strictly equal; false otherwise.
	 */
	bool equal(const write_dict_string_t& lhs, const write_dict_string_t& rhs) const
	{
		return cstring_compare(lhs.value(), rhs.value());
	}

	/**
	 * Computes the hash given a key value
	 *
	 * @return the hash of @k k
	 */
	size_t hash(const write_dict_string_t& k) const { return cstring_hash(k.value()); }
};

} // namespace pvcop::db::__impl

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_IMPL_WRITE_DICT_TYPES_H__
