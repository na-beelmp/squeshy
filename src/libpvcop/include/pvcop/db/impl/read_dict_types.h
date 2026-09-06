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

#ifndef __PVCOP_DB_IMPL_READ_DICT_TYPES_H__
#define __PVCOP_DB_IMPL_READ_DICT_TYPES_H__

#include <pvcop/db/impl/cstring_traits.h>

namespace pvcop
{

namespace db
{

namespace __impl
{

/**
 * Structure conform to tbb::tbb_hash_compare<T> to use with the internal
 * container of class pvcop::core::map_index.
 */
struct read_dict_string_hashcompare_t {
	/**
	 * Checks for the equality of 2 C strings.
	 *
	 * @param lhs the first key
	 * @param rhs the other key
	 *
	 * @return true if the 2 keys are strictly equal; false otherwise.
	 */
	bool equal(const char* lhs, const char* rhs) const { return cstring_compare(lhs, rhs); }

	/**
	 * Computes the hash of a C strings
	 *
	 * @return the hash of @c k
	 */
	size_t hash(const char* k) const { return cstring_hash(k); }
};

} // namespace pvcop::db::__impl

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_IMPL_READ_DICT_TYPES_H__
