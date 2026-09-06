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

#ifndef __PVCOP_CORE_MEMARRAY_H__
#define __PVCOP_CORE_MEMARRAY_H__

#include <pvlogger.h>

#include <pvcop/core/array.h>
#include <pvcop/core/impl/memory.h>

#include <algorithm>

namespace pvcop
{

namespace core
{

/**
 * This class extends pvcop::core::array by adding the ownership on a memory
 * area.
 */
template <typename T>
class memarray : public array<T>
{
	using __type_traits = __impl::type_traits<T>;
	using array_t = array<T>;
	using memarray_t = memarray<T>;

  public:
	using value_type = typename __type_traits::value_type;

  public:
	/**
	 * Default constructor
	 */
	memarray() : array_t()
	{
		pvlogger::trace() << "core::memarray::memarray() : " << this << std::endl;
	}

	/**
	 * Size constructor
	 *
	 * @param size the size of this array
	 */
	explicit memarray(size_t size, bool init = false)
	{
		pvlogger::trace() << "core::memarray::memarray(" << size << ") : " << this << std::endl;

		array_t::_data = __impl::mem_allocate<value_type>(__type_traits::to_mem_size(size), init);
		array_t::_size = size;
	}

	/**
	 * No copy constructor
	 */
	memarray(memarray_t const& array) = delete;

	memarray(array_t const& array)
	{
		pvlogger::trace() << "core::memarray::memarray(array_t const& array) : " << this << " <- "
		                  << &array << std::endl;

		if (array) {
			array_t::_data = __impl::mem_allocate<value_type>(array.mem_size(), false);
			array_t::_size = array.size();
			pvlogger::trace() << "array.data()=" << array.data() << " array.size()=" << array.size()
			                  << " array.mem_size()=" << array.mem_size() << std::endl;
			array_t::copy_from(array, 0, array.size());
		} else {
			array_t::_data = nullptr;
			array_t::_size = 0;
		}
	}

	/**
	 * Move constructor
	 *
	 * @param array the memarray to move from
	 */
	memarray(memarray_t&& array)
	{
		pvlogger::trace() << "core::memarray::memarray(memarray_t&&) : " << this << std::endl;

		if (array_t::_data != nullptr) {
			__impl::mem_deallocate<value_type>(array_t::_data);
		}
		array_t::_data = array.array_t::_data;
		array_t::_size = array.array_t::_size;
		array.array_t::_data = nullptr;
		array.array_t::_size = 0;
	}

	/**
	 * Slice constructor from an other array
	 *
	 * @param array the array to copy from
	 * @param abs_pos the start position in @a array
	 * @param size the size of the new array
	 */
	memarray(const array_t& array, size_t abs_pos, size_t size) : memarray_t()
	{
		pvlogger::trace() << "core::memarray::memarray(" << abs_pos << ", " << size
		                  << ") : " << this << std::endl;

		assert(__type_traits::is_pos_valid(abs_pos));

		if (abs_pos >= array.array_t::_size) {
			return;
		}

		if ((abs_pos + size) > array.array_t::_size) {
			return;
		}

		memarray(array.array_t::_data, abs_pos, size);
	}

	/**
	 * Slice constructor from a raw pointer
	 *
	 * @param data the raw pointer to copy from
	 * @param abs_pos the start position in @a data
	 * @param size the size of the new array
	 */
	memarray(const value_type* data, size_t abs_pos, size_t size) : memarray_t()
	{
		pvlogger::trace() << "core::memarray::memarray(..., " << abs_pos << ", " << abs_pos
		                  << ") : " << this << std::endl;

		assert(__type_traits::is_pos_valid(abs_pos));

		array_t::_data = __impl::mem_allocate<value_type>(__type_traits::to_mem_size(size), false);
		array_t::_size = size;
		array_t::copy_from(data, abs_pos, size);
	}

	/**
	 * Destructor
	 */
	~memarray()
	{
		pvlogger::trace() << "core::memarray::~memarray() : " << this << std::endl;

		if (array_t::_data != nullptr) {
			__impl::mem_deallocate<value_type>(array_t::_data);
			array_t::_data = nullptr;
			array_t::_size = 0;
		}
	}

  public:
	/**
	 * No copy assignment operator
	 */
	memarray_t& operator=(memarray_t const& rhs) = delete;

	memarray_t& operator=(array_t const& rhs)
	{
		pvlogger::trace() << "core::memarray::operator=(array_t const& rhs)" << std::endl;

		if (this != &rhs) {
			if (array_t::_data != nullptr) {
				__impl::mem_deallocate<value_type>(array_t::_data);
			}
			if (rhs) {
				array_t::_data = __impl::mem_allocate<value_type>(rhs.mem_size(), false);
				array_t::_size = rhs.size();
				array_t::copy_from(rhs, 0, rhs.size());
			} else {
				array_t::_data = nullptr;
				array_t::_size = 0;
			}
		}

		return *this;
	}

	/**
	 * Move assignment
	 *
	 * @param rhs the memarray to move from
	 *
	 * @return a reference on itself
	 */
	memarray_t& operator=(memarray_t&& rhs)
	{
		pvlogger::trace() << "core::memarray::operator=(memarray_t&& rhs)" << std::endl;

		if (array_t::_data != nullptr) {
			__impl::mem_deallocate<value_type>(array_t::_data);
		}

		array_t::_data = rhs.array_t::_data;
		array_t::_size = rhs.array_t::_size;
		rhs.array_t::_data = nullptr;
		rhs.array_t::_size = 0;

		return *this;
	}

  public:
	/**
	 * Shrink memarray if it was created with too much memory
	 *
	 * @note It may work to increase size but it is not tested for it.
	 */
	void resize(size_t new_size)
	{
		assert(new_size <= array_t::_size && "Resize should only be use to shrink container");

		array_t::_data = __impl::mem_reallocate<value_type>(array_t::_data,
		                                                    __type_traits::to_mem_size(new_size));
		array_t::_size = new_size;
	}
};

} // namespace pvcop::core

} // namespace pvcop

#include <pvcop/core/algo/bitwise_operators.h>

#endif // __PVCOP_CORE_MEMARRAY_H__
