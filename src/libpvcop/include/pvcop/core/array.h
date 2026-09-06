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

#ifndef __PVCOP_CORE_ARRAY_H__
#define __PVCOP_CORE_ARRAY_H__

#include <pvlogger.h>

#include <pvcop/core/impl/type_traits.h>

#include <cassert>
#include <stdexcept>

namespace pvcop
{

/**
 * This namespace provides the low-level containers and algorithms to work
 * efficiently with data stored as arrays.
 */
namespace core
{

/**
 * A fixed-size sequence container which has no ownership on its memory array.
 *
 * This class is an abstraction on explicitly typed data array. In addition to
 * providing usual accesses on containers, it gives ways to construct sub-array
 * to ease parallel processing (see pvcop::core::array::slice) or reconstruct
 * an array from sub-arrays (see pvcop::core::array::copy_from).
 *
 * As an abstraction, this class does not have the ownership on the memory area
 * it permits to work on. To create arrays which explicity own their memory area,
 * see pvcop::core::memarray.
 */
template <typename T>
class array
{
  public:
	static constexpr size_t INVALID_INDEX = -1;

  public:
	using __type_traits = __impl::type_traits<T>;

  public:
	using value_type = T;
	using data_type = typename __type_traits::value_type;
	using iterator = typename __type_traits::iterator;
	using const_iterator = typename __type_traits::const_iterator;

	using accessor = typename __type_traits::accessor;
	using const_accessor = typename __type_traits::const_accessor;

  public:
	/**
	 * Default constructor
	 */
	array() : _data(nullptr), _size(0)
	{
		pvlogger::trace() << "core::array::array() : " << this << std::endl;
	}

	/**
	 * Slice constructor
	 *
	 * @param array the original array
	 * @param abs_pos the start position in @a array
	 * @param size the size of the new array
	 */
	array(const array& array, size_t abs_pos, size_t size) : _data(nullptr), _size(0)
	{
		pvlogger::trace() << "core::array::array(" << abs_pos << ", " << size << ") : " << this
		                  << std::endl;

		assert(__type_traits::is_pos_valid(abs_pos));

		if (abs_pos >= array._size) {
			return;
		}

		if ((abs_pos + size) > array._size) {
			return;
		}

		_data = array._data + __type_traits::pos_to_offset(abs_pos);
		_size = size;
	}

	/**
	 * Destructor
	 */
	~array() { pvlogger::trace() << "core::array::~array() : " << this << std::endl; }

  public:
	/**
	 * Retrieve array size in element
	 *
	 * @return the array size in element
	 */
	inline size_t size() const { return _size; }

	/**
	 * Retrieve pointer on raw data
	 *
	 * @return the pointer on the array data
	 */
	inline data_type* data() { return _data; }

	/**
	 * Retrieve pointer on raw data
	 *
	 * @return the pointer on the array data
	 */
	inline const data_type* data() const { return _data; }

	/**
	 * Retrieve array memory size
	 *
	 * @return the array memory size
	 */
	inline size_t mem_size() const { return __type_traits::to_mem_size(_size); }

  public:
	/**
	 * Random access with range check
	 *
	 * @param i the value index
	 *
	 * @return a reference on the ith value if @a i is a valid index; an @b out_of_range exception
	 *is
	 *throwed otherwise
	 */
	inline accessor at(size_t i)
	{
		if (i >= _size) {
			throw std::out_of_range("invalid index for pvcop::core::array::at(size_t)");
		}

		return __type_traits::get_accessor(_data, i);
	}

	/**
	 * Random access with range check
	 *
	 * @param i the value index
	 *
	 * @return a const reference to the ith value if @a i is a valid index; an @b out_of_range
	 *exception is thrown
	 *otherwise
	 */
	inline const_accessor at(size_t i) const
	{
		if (i >= _size) {
			throw std::out_of_range("invalid index for pvcop::core::array::at(size_t)");
		}

		return __type_traits::get_accessor(_data, i);
	}

	/**
	 * Random access without range check
	 *
	 * @param i the value index
	 *
	 * @return a reference on the ith value
	 */
	inline accessor operator[](size_t i)
	{
		assert(i < _size);
		return __type_traits::get_accessor(_data, i);
	}

	/**
	 * Random access without range check
	 *
	 * @param i the value index
	 *
	 * @return a const reference on the ith value
	 */
	inline const_accessor operator[](size_t i) const
	{
		assert(i < _size);
		return __type_traits::get_accessor(_data, i);
	}

  public:
	bool operator==(const array& other) const { return std::equal(begin(), end(), other.begin()); }

	bool operator!=(const array& other) const { return !(*this == other); }

  public:
	/**
	 * STL compatible non-const ::begin()
	 *
	 * @return an iterator referring to the first value
	 */
	inline iterator begin() { return __type_traits::get_iterator(_data, 0); }

	/**
	 * STL compatible const ::begin()
	 *
	 * @return a const iterator referring to the first value
	 */
	inline const_iterator begin() const { return __type_traits::get_const_iterator(_data, 0); }
	inline const_iterator cbegin() const { return __type_traits::get_const_iterator(_data, 0); }

	/**
	 * STL compatible non-const ::end()
	 *
	 * @return an iterator referring to the past-the-end element
	 */
	inline iterator end() { return __type_traits::get_iterator(_data, _size); }

	/**
	 * STL compatible const ::end()
	 *
	 * @return a const iterator referring to the past-the-end value
	 */
	inline const_iterator end() const { return __type_traits::get_const_iterator(_data, _size); }
	inline const_iterator cend() const { return __type_traits::get_const_iterator(_data, _size); }

  public:
	/**
	 * Convert to bool to check array validity
	 *
	 * @return @c true if the bit array is valid; @c false otherwise
	 *
	 * @fixme : should be explicit
	 */
	inline operator bool() const { return (_data != nullptr); }

  public:
	/**
	 * Get a slice from an array
	 *
	 * @param abs_pos the starting index
	 * @param size the length of the final slice
	 *
	 * @return an array on [abs_pos,abs_pos+size[ if parameters match array geometry; a null array
	 *otherwise.
	 */
	array slice(size_t abs_pos, size_t size) { return array(*this, abs_pos, size); }

	/**
	 * Get a slice from an array
	 *
	 * @param abs_pos the starting index
	 * @param size the length of the final slice
	 *
	 * @return an array on [abs_pos,abs_pos+size[ if parameters match array geometry; a null array
	 *otherwise.
	 */
	const array slice(size_t abs_pos, size_t size) const { return array(*this, abs_pos, size); }

  public:
	/**
	 * Fill with data from an other array
	 *
	 * @param array the array to copy from
	 * @param abs_pos the start position in @a array
	 * @param size the number of elements to copy
	 *
	 * @return true is the copy is successful; false otherwise
	 */
	bool copy_from(const array& array, size_t abs_pos, size_t size)
	{
		return copy_from(array._data, abs_pos, size);
	}

	/**
	 * Fill with data from a raw buffer
	 *
	 * @param data the raw pointer to copy from
	 * @param abs_pos the start position in @a data
	 * @param size the number of elements to copy
	 *
	 * @return true is the copy is successful; false otherwise
	 */
	bool copy_from(const data_type* data, size_t abs_pos, size_t size)
	{
		assert(__type_traits::is_pos_valid(abs_pos));

		if (abs_pos >= _size) {
			return false;
		}

		if ((abs_pos + size) > _size) {
			return false;
		}

		data_type* out_addr = _data + __type_traits::pos_to_offset(abs_pos);

		std::copy(data, data + __type_traits::to_mem_size(size) / sizeof(*data), out_addr);

		return true; // FIXME !!
	}

  public:
	data_type* _data = nullptr;
	size_t _size = 0;
};

} // namespace core

} // namespace pvcop

#endif // __PVCOP_CORE_ARRAY_H__
