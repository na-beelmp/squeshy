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

#ifndef __PVCOP_CORE__IMPL_SEL_ITERATOR_H__
#define __PVCOP_CORE__IMPL_SEL_ITERATOR_H__

#include <pvcop/core/algo/selection.h>
#include <pvcop/core/impl/bit.h>

#include <pvlogger.h>

#include <algorithm>
#include <iterator>

namespace pvcop
{

namespace core
{

namespace __impl
{

template <typename T>
struct type_traits;

template <typename T>
using sel_iterator_value_type = typename type_traits<T>::value_type;

/**
 * Abstraction to iterate over arrays using a bitfield selection (core::selection)
 *
 * This class provides both an iterator and a const_iterator.
 */
template <typename T, bool IsConst>
class sel_iterator_impl
{
  public:
  	using iterator_category = std::forward_iterator_tag;
	using difference_type = std::ptrdiff_t;
    using pointer = typename std::conditional<IsConst,
	                                          const sel_iterator_value_type<T>,
	                                          sel_iterator_value_type<T>>::type*;
    using reference = typename std::conditional<IsConst,
	                                          const sel_iterator_value_type<T>,
	                                          sel_iterator_value_type<T>>::type&;
	using value_type = typename std::conditional<IsConst,
	                                             const sel_iterator_value_type<T>,
	                                             sel_iterator_value_type<T>>::type;
	using accessor = typename std::conditional<IsConst,
	                                           typename type_traits<T>::const_accessor,
	                                           typename type_traits<T>::accessor>::type;

  public:
	/**
	 * Constructor
	 *
	 * @param data pointer to the array internal buffer.
	 * @param sel pointer (or nullptr) to the selection internal buffer.
	 * @param index index of the starting element
	 * @param size iterator elements count
	 */
	sel_iterator_impl(value_type* data,
	                  const bit_manip::value_type* sel,
	                  const size_t index,
	                  const size_t size)
	    : _data(data), _sel(sel), _size(size), _index(index)

	{
		assert(data);

		_index = find_first_set_bit(_index, _size);
	}

	/**
	 * Pre-increment operator
	 */
	sel_iterator_impl& operator++()
	{
		_index = find_first_set_bit(_index + 1, _size);

		return *this;
	}

	/**
	 * Post-increment operator
	 */
	sel_iterator_impl& operator++(int)
	{
		sel_iterator_impl temp = *this;
		++*this;

		return temp;
	}

	/**
	 * Pre-increment operator
	 */
	sel_iterator_impl& operator+=(int64_t n)
	{
		if (n == 1) {
			// Special shortcut as it is the most called value with OpenMP and it is really faster.
			return ++(*this);
		}
		if (n == 0) {
			return *this;
		}

		_index = find_nth_set_bit(n, _index + 1, _size);
		assert(_index == (size_t)-1 or _index < _size);

		return *this;
	}

	inline sel_iterator_impl operator+(int64_t n)
	{
		sel_iterator_impl temp = *this;

		temp._index = find_nth_set_bit(n, _index + 1, _size);

		return temp;
	}

	/**
	 * Pre-decrement operator
	 */
	sel_iterator_impl& operator--()
	{
		_index = find_first_set_bit(_index - 1, _size);

		return *this;
	}

	/**
	 * Post-decrement operator
	 */
	sel_iterator_impl& operator--(int)
	{
		sel_iterator_impl temp = *this;
		--*this;

		return temp;
	}

	/**
	 * Pre-decrement operator
	 */
	sel_iterator_impl& operator-=(int64_t n)
	{
		_index = find_nth_last_set_bit(n, _index + 1, _size);

		return *this;
	}

	int64_t operator-(sel_iterator_impl const& rhs) const
	{
		if (_index == (size_t)-1) {
			return pvcop::core::algo::bit_count(_sel, rhs._index, _size - 1);
		}
		if (rhs._index == (size_t)-1) {
			return rhs - *this;
		}
		return _index - rhs._index;
	}

	/**
	 * Iterator equality test
	 *
	 * @param rhs the iterator to compare to
	 *
	 * @return @c true if the iterators are identical; @c false otherwise
	 */
	bool operator==(const sel_iterator_impl& rhs) const
	{
		return _data == rhs._data && _sel == rhs._sel && _index == rhs._index;
	}

	/**
	 * Iterator inequality test
	 *
	 * @param rhs the iterator to compare to
	 *
	 * @return @c true if the iterators are different; @c false otherwise
	 */
	bool operator!=(const sel_iterator_impl& rhs) const { return !(*this == rhs); }

	bool operator<(const sel_iterator_impl& rhs) const { return _index < rhs._index; }
	bool operator<=(const sel_iterator_impl& rhs) const { return _index <= rhs._index; }
	bool operator>(const sel_iterator_impl& rhs) const { return _index > rhs._index; }
	bool operator>=(const sel_iterator_impl& rhs) const { return _index >= rhs._index; }

	/**
	 * Dereference operator
	 *
	 * @return @c the value stored at the iterator current location
	 */
	accessor operator*() const { return type_traits<T>::get_accessor(_data, index()); }
	pointer operator->() const { return &type_traits<T>::get_accessor(_data, index()); }

	inline size_t find_first_set_bit(const size_t start, const size_t end) const
	{
		if (_sel == nullptr) {
			return start;
		}

		// The public core::algo::find_first_set_bit() clamps end to size() - 1 before
		// reaching the implementation, because end is inclusive there and the selection
		// only holds _size bits. Going through __impl directly has to clamp too, or the
		// end iterator (start == _size) reads a chunk past the buffer, and a stray bit in
		// that memory yields an index beyond the last element.
		if (start >= _size) {
			return -1; // core::selection::INVALID_INDEX
		}

		return core::algo::__impl::find_first_set_bit(_sel, start, std::min(_size - 1, end));
	}

	inline size_t find_nth_set_bit(int64_t n, const size_t start, const size_t end) const
	{
		if (_sel == nullptr) {
			if (start + n > end) {
				return -1;
				// core::selection::INVALID_INDEX;
			}
			return start + n;
		}

		if (start >= _size) {
			return -1; // core::selection::INVALID_INDEX
		}

		return core::algo::__impl::find_nth_set_bit(_sel, n, start, std::min(_size - 1, end));
	}

	inline size_t find_nth_last_set_bit(int64_t n, const size_t start, const size_t end) const
	{
		if (_sel == nullptr) {
			if (end - n < start) {
				return -1;
				// core::selection::INVALID_INDEX;
			}
			return end - n;
		}

		if (_size == 0) {
			return -1; // core::selection::INVALID_INDEX
		}

		return core::algo::__impl::find_nth_last_set_bit(_sel, n, start, std::min(_size - 1, end));
	}

	/**
	 * Iterator current location index
	 *
	 * @return @c the index of the iterator current location
	 */
	inline size_t index() const { return _index; }

  private:
	value_type* _data;
	const bit_manip::value_type* _sel;
	size_t _size;
	size_t _index;
};

} // namespace pvcop::core::__impl

} // namespace pvcop::core

} // namespace pvcop

#endif // __PVCOP_CORE__IMPL_SEL_ITERATOR_H__
