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

#ifndef __PVCOP_CORE_MAP_INDEX_H__
#define __PVCOP_CORE_MAP_INDEX_H__

#include <atomic>

#include <limits>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <tbb/concurrent_hash_map.h>
#pragma GCC diagnostic pop

namespace pvcop
{

namespace core
{

/**
 * A template class to associate concurrently a unique number (of type @p Index) for each element
 * (of type @p Key) of a values set.
 *
 * The numbers match the order in which the values are inserted; they have to be considered as
 * insertion index, not as random access index. The values set does not require to be ordered and
 * the keys are not sorted.
 *
 * This template class can be considered as an unordered map where the value item stores the
 * associated index.
 *
 * This template class can be used in different ways:
 * - using insert() to build the distinct values set of a given data set;
 * - using insert() to build an index based data set equivalent to the original data set;
 * - using exist() to check that a key belongs to the distinct values set or not;
 * - using find() to retrieve the entry corresponding to a given key;
 * - using index() to retrieve the index associated to a given key;
 * - to iterate over the content to process the pairs (key, index).
 *
 * @note the insertion performance greatly depends on the number of possible
 * distinct values you want to store in this class instance.
 *
 * @note as this template class depends on tbb::concurrent_hash_map, the template parameter
 * @p HashCompare must match the TBB HashCompare concept.
 */
template <typename Key, typename Index, typename HashCompare = tbb::tbb_hash_compare<Key>>
class map_index
{
  private:
	using map_t = tbb::concurrent_hash_map<Key, Index, HashCompare>;

  public:
	/**
	 * Key.
	 */
	using key_type = Key;

	/**
	 * Index.
	 */
	using index_type = Index;

	/**
	 * size_t
	 */
	using size_type = size_t;

	/**
	 * A pair composed of @a Key and @a Index.
	 */
	using value_type = typename map_t::const_accessor::value_type;

	/**
	 * Constant random access iterator
	 */
	using const_iterator = typename map_t::const_iterator;

  public:
	/**
	 * Constant representing an index value which does not exist.
	 */
	static constexpr index_type invalid_index = std::numeric_limits<index_type>::max();

  public:
	/**
	 * Constructor
	 */
	map_index() : _next_index(0) {}

  public:
	/**
	 * Inserts a key, if there is no corresponding entry.
	 *
	 * @param key the key to insert
	 *
	 * @return the insertion index of the entry corresponding to @p key.
	 *
	 * @note this method is thread-safe.
	 *
	 * @note this method is not deterministic in a multi-threaded context.
	 */
	index_type insert(const key_type& key)
	{
		typename map_t::const_accessor cacc;
		typename map_t::accessor acc;

		/* optimistic search: a multiple read access
		 */
		if (_map.find(cacc, key)) {
			return cacc->second;
		}
		cacc.release();

		/* pessimistic search: a single read/write access
		 */
		if (_map.insert(acc, key)) {
			acc->second = _next_index.fetch_add(1);
		}

		return acc->second;
	}

	/**
	 * Checks if an entry equivalent to @p key exists or not.
	 *
	 * @param key the key to test
	 *
	 * @return true if such an entry exists; false otherwise.
	 *
	 * @note the returned value correctness is not garanteed when insert() is called
	 * concurrently.
	 */
	bool exist(const key_type& key) const
	{
		typename map_t::const_accessor cacc;

		return _map.find(cacc, key);
	}

	/**
	 * Tries to retrieve the entry corresponding to @p key.
	 *
	 * @param key the key to retrieve
	 *
	 * @return the entry corresponding to @p key if it exists; an entry with an invalid index
	 * otherwise.
	 *
	 * @note the returned value correctness is not garanteed when insert() is called
	 * concurrently.
	 */
	value_type find(const key_type& key) const
	{

		typename map_t::const_accessor cacc;

		if (_map.find(cacc, key)) {
			return *cacc;
		} else {
			/* GCC and Clang fail at link time (undefined reference) if ::invalid_index
			 * is directly used as entry_t second parameter; using an intermediate
			 * variable seems to be an adequate work-around.
			 */
			auto iindex = invalid_index;
			return value_type(key_type(), iindex);
		}
	}

  public:
	/**
	 * Tries to retrieve the index of the entry equivalent to @p key.
	 *
	 * @param key the key whose we want the index
	 *
	 * @return the equivalent entry index if such an entry exists; @b invalid_index otherwise.
	 *
	 * @note the returned value correctness is not garanteed when insert() is called
	 * concurrently.
	 */
	index_type index(const key_type& key) const
	{
		typename map_t::const_accessor cacc;

		if (_map.find(cacc, key)) {
			return cacc->second;
		} else {
			return invalid_index;
		}
	}

  public:
	/**
	 * Gets the number of stored entry.
	 *
	 * @return the number of stored entry.
	 *
	 * @note the returned value correctness is not garanteed when insert() is called
	 * concurrently.
	 */
	size_type size() const { return _map.size(); }

  public:
	/**
	 * Sets the number of buckets to @p count and rehashes the underlying container.
	 *
	 * @param count the wanted buckets count
	 *
	 * @warning this method must not be concurrently invoked
	 */
	void rehash(size_type count = 0) { _map.rehash(count); }

	/**
	 * Removes all entries.
	 */
	void clear() { _map.clear(); }

  public:
	/**
	 * Gets a const iterator to the beginning of this index.
	 *
	 * @return a const iterator to the beginning.
	 *
	 * @warning calls to insert() will invalidate any existing iterators.
	 */
	const_iterator begin() const { return _map.begin(); }

	/**
	 * Gets a const iterator on the past the end of this index.
	 *
	 * @return a const iterator on the past the end.
	 *
	 * @warning calls to insert() will invalidate any existing iterators.
	 */
	const_iterator end() const { return _map.end(); }

  private:
	map_t _map;
	std::atomic<index_type> _next_index;
};

} // namespace pvcop::core

} // namespace pvcop

#endif // __PVCOP_CORE_MAP_INDEX_H__
