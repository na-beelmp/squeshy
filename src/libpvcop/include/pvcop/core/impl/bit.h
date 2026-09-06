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

#ifndef __PVCOP_CORE_BIT_H__
#define __PVCOP_CORE_BIT_H__

#include <cstddef>
#include <cassert>
#include <iterator>
#include <cstdint>

namespace pvcop
{

namespace core
{

namespace __impl
{

namespace bit_manip
{
#ifdef __linux__
using value_type = unsigned long;
#else
using value_type = uint64_t;
#endif
/**
 * The chunk size in byte
 */
static constexpr size_t chunk_byte_size = sizeof(value_type);

/**
 * The chunk size in bit
 */
static constexpr size_t chunk_bit_size = chunk_byte_size * 8;

/**
 * Get a chunk index from an absolute index
 *
 * @param abs_index the index in the bit array of the wanted bit
 *
 * @return the chunk index corresponding to abs_index
 */
inline static size_t chunk_index(size_t abs_index)
{
	return abs_index / chunk_bit_size;
}

/**
 * Get a bit index from an absolute index
 *
 * @param abs_index the index in the bit array of the wanted bit
 *
 * @return the bit index corresponding to abs_index
 */
inline static size_t bit_index(size_t abs_index)
{
	return abs_index % chunk_bit_size;
}

/**
 * Get the number of chunks needed to store a given number of bit
 *
 * @param size the number of bits
 *
 * @return the number of chunks to store count bits
 */
inline static constexpr size_t chunk_count(size_t size)
{
	return (size + (chunk_bit_size - 1)) / chunk_bit_size;
}

/**
 * Get the memory usage needed to store a given number of bit
 *
 * @param size the number of bits
 *
 * @return the memory size to store count bits
 */
inline static constexpr size_t to_mem_size(size_t size)
{
	return chunk_count(size) * chunk_byte_size;
}

inline static constexpr size_t from_mem_size(size_t size)
{
	return size * chunk_byte_size;
}

inline static bool is_selected(value_type const* base_addr, const size_t abs_index)
{
	return *(base_addr + chunk_index(abs_index)) & ((value_type)1 << bit_index(abs_index));
}
} // namespace bit_manip

class bit
{
  public:
	/**
	 * Default constructor
	 *
	 * @param base_addr the base address of the bit array
	 * @param abs_index the index in the bit array of the wanted bit
	 */
	bit(bit_manip::value_type* base_addr, const size_t abs_index)
	{
		_chunk = base_addr + bit_manip::chunk_index(abs_index);
		_index = bit_manip::bit_index(abs_index);
	}

  public:
	/**
	 * Get the bit mask
	 *
	 * @return the bit mask
	 */
	inline size_t mask() const { return (size_t)1 << _index; }

	/**
	 * Get the bit index in its chunk
	 *
	 * @return the bit index in its chunk
	 */
	inline size_t index() const { return _index; }

	/**
	 * Get the pointer on the bit chunk
	 *
	 * @return the pointer on the bit chunk
	 */
	inline bit_manip::value_type* chunk() { return _chunk; }
	inline const bit_manip::value_type* chunk() const { return _chunk; }

  public:
	/**
	 * Bit assignment from a bool
	 *
	 * @code{.cpp}
	 * a[i] = true|false;
	 * @endcode
	 *
	 * @param rhs the value to assign
	 *
	 * @return a reference on this
	 */
	inline bit& operator=(bool rhs)
	{
		assert(_chunk != nullptr);

		if (rhs) {
			*_chunk |= mask();
		} else {
			*_chunk &= ~mask();
		}

		return *this;
	}

	/**
	 * Bit assignment from a bit
	 *
	 * @code{.cpp}
	 * a[i] = rhs[j];
	 * @endcode
	 *
	 * @param rhs the bit to assign
	 *
	 * @return a reference on this
	 */
	inline bit& operator=(const bit& rhs)
	{
		assert(_chunk != nullptr);
		assert(rhs._chunk != nullptr);

		if (*rhs._chunk & rhs.mask()) {
			*_chunk |= mask();
		} else {
			*_chunk &= ~mask();
		}

		return *this;
	}

  public:
	/**
	 * Bits equality test
	 *
	 * @param rhs the bit to compare to
	 *
	 * @return @c true if it refer to the same bit; @c false otherwise
	 */
	inline bool operator==(const bit& rhs) const
	{
		return (_chunk == rhs._chunk) && (_index == rhs._index);
	}

	/**
	 * Bits inequality test
	 *
	 * @param rhs the bit to compare to
	 *
	 * @return @c true if it refer to different bit; @c false otherwise
	 */
	inline bool operator!=(const bit& rhs) const { return not(*this == rhs); }

  public:
	/**
	 * Bit conversion to bool
	 *
	 * @return a bit value
	 */
	inline operator bool() const { return bit_manip::is_selected(_chunk, _index); }

  private:
	bit_manip::value_type* _chunk = nullptr;
	size_t _index;
};

class const_bit_iterator
{
  public:
  	using iterator_category = std::random_access_iterator_tag;
    using value_type = bool;
	using difference_type = int;
    using pointer = int*;
    using reference = int&;
	/**
	 * Default constructor
	 *
	 * @param base_addr the base address of the bit array
	 * @param abs_index the index in the bit array of the wanted bit
	 */
	const_bit_iterator(const bit_manip::value_type* base_addr, const size_t abs_index)
	    : _chunk(base_addr + bit_manip::chunk_index(abs_index))
	    , _index(bit_manip::bit_index(abs_index))
	{
	}

  public:
	/**
	 * Get the bit index in its chunk
	 *
	 * @return the bit index in its chunk
	 */
	inline size_t index() const { return _index; }

	/**
	 * Get the pointer on the bit chunk
	 *
	 * @return the pointer on the bit chunk
	 */
	inline const bit_manip::value_type* chunk() const { return _chunk; }

  public:
	/**
	 * Bits equality test
	 *
	 * @param rhs the bit to compare to
	 *
	 * @return @c true if it refer to the same bit; @c false otherwise
	 */
	inline bool operator==(const const_bit_iterator& rhs) const
	{
		return (_chunk == rhs._chunk) && (_index == rhs._index);
	}

	/**
	 * Bits inequality test
	 *
	 * @param rhs the bit to compare to
	 *
	 * @return @c true if it refer to different bit; @c false otherwise
	 */
	inline bool operator!=(const const_bit_iterator& rhs) const { return not(*this == rhs); }

  public:
	/**
	 * Bit index increment
	 *
	 * @return a reference on this
	 *
	 * @warning no boundary check is done
	 */
	inline const_bit_iterator& operator++()
	{
		++_index;
		if (_index == bit_manip::chunk_bit_size) {
			++_chunk;
			_index = 0;
		}
		return *this;
	}

	/**
	 * Bit index addition
	 *
	 * @return a bit translate from @a value
	 *
	 * @warning no boundary check is done
	 */
	inline const_bit_iterator operator+(size_t value) const { return {_chunk, _index + value}; }

	/**
	 * Bit index sub
	 *
	 * @return a bit translate from @a value
	 *
	 * @warning no boundary check is done
	 */
	inline const_bit_iterator operator-(size_t value) const
	{
		size_t n_chunk = bit_manip::chunk_count(value);
		size_t n_value = bit_manip::chunk_index(value);
		return {_chunk - n_chunk - 1, bit_manip::chunk_bit_size + _index - n_value};
	}

	inline int64_t operator-(const_bit_iterator const& rhs) const
	{
		return (_chunk - rhs._chunk) * bit_manip::chunk_bit_size + _index - rhs._index;
	}

	inline const_bit_iterator& operator--() { return *this = *this - 1; }

	inline const_bit_iterator operator--(int)
	{
		const_bit_iterator temp = *this;
		--*this;
		return temp;
	}

	/**
	 * Bit index advance
	 *
	 * @return a reference on this
	 *
	 * @warning no boundary check is done
	 */
	inline const_bit_iterator& operator+=(size_t value)
	{
		_chunk += bit_manip::chunk_index(_index + value);
		_index = bit_manip::bit_index(_index + value);
		return *this;
	}

  public:
	/**
	 * Bit dereference
	 *
	 * @return a pointer of the bit
	 */
	inline bool operator*() const { return *_chunk & (static_cast<size_t>(1) << _index); }

  private:
	bit_manip::value_type const* _chunk = nullptr;
	size_t _index;
};

/**
 * an abraction to manipulate bits.
 */
class bit_iterator
{
  public:
  	using iterator_category = std::forward_iterator_tag;
    using value_type = bit;
	using difference_type = int;
    using pointer = int*;
    using reference = int&;

	/**
	 * Default constructor
	 *
	 * @param base_addr the base address of the bit array
	 * @param abs_index the index in the bit array of the wanted bit
	 */
	bit_iterator(bit_manip::value_type* base_addr, const size_t abs_index)
	{
		_chunk = base_addr + bit_manip::chunk_index(abs_index);
		_index = bit_manip::bit_index(abs_index);
	}

  public:
	/**
	 * Get the bit index in its chunk
	 *
	 * @return the bit index in its chunk
	 */
	inline size_t index() const { return _index; }

	/**
	 * Get the pointer on the bit chunk
	 *
	 * @return the pointer on the bit chunk
	 */
	inline bit_manip::value_type* chunk() { return _chunk; }
	inline const bit_manip::value_type* chunk() const { return _chunk; }

  public:
	/**
	 * Bits equality test
	 *
	 * @param rhs the bit to compare to
	 *
	 * @return @c true if it refer to the same bit; @c false otherwise
	 */
	inline bool operator==(const bit_iterator& rhs) const
	{
		return (_chunk == rhs._chunk) && (_index == rhs._index);
	}

	/**
	 * Bits inequality test
	 *
	 * @param rhs the bit to compare to
	 *
	 * @return @c true if it refer to different bit; @c false otherwise
	 */
	inline bool operator!=(const bit_iterator& rhs) const { return not(*this == rhs); }

  public:
	/**
	 * Bit index increment
	 *
	 * @return a reference on this
	 *
	 * @warning no boundary check is done
	 */
	inline bit_iterator& operator++()
	{
		++_index;
		if (_index == bit_manip::chunk_bit_size) {
			++_chunk;
			_index = 0;
		}
		return *this;
	}

	/**
	 * Bit index addition
	 *
	 * @return a bit translate from @a value
	 *
	 * @warning no boundary check is done
	 */
	inline bit_iterator operator+(size_t value) const { return {_chunk, _index + value}; }

	/**
	 * Bit index advance
	 *
	 * @return a reference on this
	 *
	 * @warning no boundary check is done
	 */
	inline bit_iterator& operator+=(size_t value)
	{
		_chunk += bit_manip::chunk_index(_index + value);
		_index = bit_manip::bit_index(_index + value);
		return *this;
	}

  public:
	/**
	 * Bit dereference
	 *
	 * @return a pointer of the bit
	 */
	inline bit operator*() const { return {_chunk, _index}; }

  private:
	bit_manip::value_type* _chunk = nullptr;
	size_t _index;
};

} // namespace __impl

} // namespace core

} // namespace pvcop

#endif // __PVCOP_CORE_BIT_H__
