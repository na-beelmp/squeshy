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

#ifndef __PVCOP_CORE_TYPE_TRAITS_H__
#define __PVCOP_CORE_TYPE_TRAITS_H__

#include <pvcop/core/impl/bit.h>
#include <pvcop/core/impl/sel_iterator.h>
#include <pvcop/utils/type_tuple.h>
#include <functional>

#include <cstdint>
#include <tuple>

namespace pvcop
{

namespace core
{

namespace __impl
{

template <typename T>
struct type_traits {
	using value_type = T;

	using iterator = T*;
	using const_iterator = const T*;

	using sel_iterator = sel_iterator_impl<T, false>;
	using const_sel_iterator = sel_iterator_impl<T, true>;

	using accessor = T&;
	using const_accessor = const T&;

	static constexpr size_t to_mem_size(const size_t size) { return size * sizeof(T); }

	static size_t from_mem_size(const size_t size)
	{
		assert((size % sizeof(T)) == 0);
		return size / sizeof(T);
	}

	static bool is_pos_valid(size_t) { return true; }

	static size_t pos_to_offset(size_t pos) { return pos; }

	static iterator get_iterator(value_type* base_addr, const size_t abs_pos)
	{
		return base_addr + abs_pos;
	}

	static const_iterator get_const_iterator(value_type const* base_addr, const size_t abs_pos)
	{
		return base_addr + abs_pos;
	}

	static accessor get_accessor(value_type* base_addr, const size_t abs_pos)
	{
		return base_addr[abs_pos];
	}

	static const_accessor get_accessor(const value_type* base_addr, const size_t abs_pos)
	{
		return base_addr[abs_pos];
	}

	static sel_iterator_impl<T, false> get_sel_iterator(value_type* data,
	                                                    const bit_manip::value_type* sel,
	                                                    const size_t index,
	                                                    const size_t size)
	{
		return sel_iterator_impl<T, false>(data, sel, index, size);
	}

	static sel_iterator_impl<T, true> get_const_sel_iterator(const value_type* data,
	                                                         const bit_manip::value_type* sel,
	                                                         const size_t index,
	                                                         const size_t size)
	{
		return sel_iterator_impl<T, true>(data, sel, index, size);
	}
};

template <>
struct type_traits<bool> {
	using value_type = bit_manip::value_type;

	using iterator = bit_iterator;
	using const_iterator = const_bit_iterator;

	using sel_iterator = sel_iterator_impl<bool, false>;
	using const_sel_iterator = sel_iterator_impl<bool, true>;

	using accessor = bit;
	using const_accessor = bool;

	static constexpr size_t to_mem_size(const size_t size) { return bit_manip::to_mem_size(size); }

	static size_t from_mem_size(const size_t size) { return bit_manip::from_mem_size(size); }

	static bool is_pos_valid(size_t pos) { return ((pos % bit_manip::chunk_bit_size) == 0); }

	static size_t pos_to_offset(size_t pos) { return pos / bit_manip::chunk_bit_size; }

	static iterator get_iterator(value_type* base_addr, const size_t abs_pos)
	{
		return {base_addr, abs_pos};
	}

	static const_iterator get_const_iterator(const value_type* base_addr, const size_t abs_pos)
	{
		return {base_addr, abs_pos};
	}

	static accessor get_accessor(value_type* base_addr, const size_t abs_pos)
	{
		return {base_addr, abs_pos};
	}

	static const_accessor get_accessor(const value_type* base_addr, const size_t abs_pos)
	{
		return bit_manip::is_selected(base_addr, abs_pos);
	}

	static sel_iterator_impl<bool, false> get_sel_iterator(value_type* data,
	                                                       const bit_manip::value_type* sel,
	                                                       const size_t index,
	                                                       const size_t size)
	{
		return sel_iterator_impl<bool, false>(data, sel, index, size);
	}

	static sel_iterator_impl<bool, true> get_const_sel_iterator(const value_type* data,
	                                                            const bit_manip::value_type* sel,
	                                                            const size_t index,
	                                                            const size_t size)
	{
		return sel_iterator_impl<bool, true>(data, sel, index, size);
	}
};

/**
 * Apply a function to every type of the type_tuple
 */

template <typename F, typename... Ts>
void visit_types(F&& func, pvcop::utils::type_tuple<Ts...>)
{
	int a[] __attribute((unused)) = {(std::ref(func)(Ts()), 0)...};
}

} // namespace pvcop::core::__impl

} // namespace pvcop::core

} // namespace pvcop

namespace std
{

/*
 * Specialize std::fill for bitfields to increase performances
 */
void fill(pvcop::core::__impl::bit_iterator begin,
          pvcop::core::__impl::bit_iterator end,
          bool value);
}

#endif // __PVCOP_CORE_TYPE_TRAITS_H__
