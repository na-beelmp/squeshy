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

#ifndef __PVCOP_DB_READ_DICT_H__
#define __PVCOP_DB_READ_DICT_H__

#include <pvcop/db/types.h>
#include <pvcop/db/impl/read_dict_types.h>

#include <pvcop/core/map_index.h>

#include <vector>
#include <string>

namespace pvcop
{

namespace db
{

/**
 * This class implements a read-only index for C strings.
 *
 * It shall be used to retrieve the string index from a db::collection or from a db::array.
 */
class read_dict
{
  public:
	using key_type = const char*;
	using index_type = pvcop::db::index_t;

	using string_set_t = std::vector<std::string>;

  private:
	using map_index_t =
	    core::map_index<key_type, index_type, __impl::read_dict_string_hashcompare_t>;
	using words_t = std::vector<key_type>;

  public:
	using value_type = map_index_t::value_type;
	using const_iterator = words_t::const_iterator;

  public:
	static constexpr index_type invalid_index = map_index_t::invalid_index;

  public:
	/**
	 * Default constructor.
	 */
	read_dict();

	/**
	 * File based dictionary constructor.
	 *
	 * Creates a dictionary by reading its content from a file
	 *
	 * @param filename the file name to load from.
	 */
	explicit read_dict(const std::string& filename);

	/**
	 * String set based dictionary constructor
	 *
	 * Creates a dictionary by reading its content from a strings set. The index of each string
	 * in the string set will be used as the internal index.
	 *
	 * @param strings the string set to use.
	 */
	explicit read_dict(const string_set_t& strings);

	/**
	 * Destructor.
	 */
	~read_dict() {}

  public:
	/**
	 * Loads the read_dict content from a file.
	 *
	 * @param filename the content filename
	 */
	void load(const std::string& filename);

	/**
	 * Fills the read_dict content from a string set.
	 *
	 * @param strings the string set
	 */
	void create(const string_set_t& strings);

  public:
	/**
	 * Tries to retrieve the entry corresponding to @p key.
	 *
	 * @param key the key to retrieve
	 *
	 * @return the entry corresponding to @p key if it exists; an entry with an invalid index
	 * otherwise.
	 */
	const value_type find(key_type key) const { return _index.find(key); }

	/**
	 * Checks if an entry equivalent to @p key exists or not.
	 *
	 * @param key the key to test
	 *
	 * @return true if such an entry exists; false otherwise.
	 */
	bool exist(key_type key) const { return _index.exist(key); }

  public:
	/**
	 * Tries to retrieve the index of the entry equivalent to @p key.
	 *
	 * @param key the key whose we want the index
	 *
	 * @return the equivalent entry index if such an entry exists; @b invalid_index otherwise.
	 */
	index_type index(key_type key) const { return _index.index(key); }

	/**
	 * Retrieves the key correspondinf to @p index
	 *
	 * @param index the index whose we want the key
	 *
	 * @return the corresponding key if such an entry exists; @b nullptr otherwise.
	 */
	key_type key(index_type index) const { return _words[index]; }

  public:
	/**
	 * Gets a const iterator to the beginning of this index.
	 *
	 * @return a const iterator to the beginning.
	 *
	 * @warning calls to insert() will invalidate any existing iterators.
	 */
	const_iterator begin() const { return _words.cbegin(); }

	/**
	 * Gets a const iterator on the past the end of this index.
	 *
	 * @return a const iterator on the past the end.
	 *
	 * @warning calls to insert() will invalidate any existing iterators.
	 */
	const_iterator end() const { return _words.cend(); }

  public:
	/**
	 * Gets the number of entries.
	 *
	 * @return the number of entries.
	 */
	size_t size() const { return _words.size(); }

  private:
	/**
	 * Finalizes internal structures initialization.
	 */
	void finalize();

  private:
	std::string _data;
	words_t _words;
	map_index_t _index;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_READ_DICT_H__
