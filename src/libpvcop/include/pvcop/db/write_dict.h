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

#ifndef __PVCOP_DB_WRITE_DICT_H__
#define __PVCOP_DB_WRITE_DICT_H__

#include <pvcop/db/types.h>
#include <pvcop/db/impl/write_dict_types.h>

#include <pvcop/core/map_index.h>

namespace pvcop
{

namespace db
{

/**
 * This class implements a insert-only index for C strings.
 *
 * It shall be used in a pvcop::db::sink instance to fill columns
 * which are typed as string.
 */
class write_dict
{
  public:
	using index_type = pvcop::db::index_t;

  private:
	using map_index_t = core::map_index<__impl::write_dict_string_t,
	                                    index_type,
	                                    __impl::write_dict_string_hashcompare_t>;
	using const_iterator = typename map_index_t::const_iterator;

  public:
	static constexpr index_type invalid_index = map_index_t::invalid_index;

  public:
	/**
	 * Constructor.
	 */
	write_dict() {}

  public:
	/**
	 * Inserts a value, if an equivalent element doesn't already exist.
	 *
	 * @param str the string to insert
	 *
	 * @return the insertion index of the key corresponding to @a str.
	 *
	 * @see pvcop::core::map_index for further informations.
	 *
	 * @note this method is thread-safe.
	 */
	index_type insert(const char* str) { return _index.insert(str); }

  public:
	/**
	 * Saves the index into a file
	 *
	 * @param filename the target filename
	 *
	 * @throw std::ios::failure in case of error
	 */
	void save(const std::string& filename) const;

  public:
	size_t size() const { return _index.size(); }

  protected:
	map_index_t _index;
};

} // namespace pvcop::db

} // namespace pvcop

#endif //  __PVCOP_DB_WRITE_DICT_H__
