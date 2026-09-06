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

#ifndef __PVCOP_DB_ARRAY_PRIMITIVE_INDEX_H__
#define __PVCOP_DB_ARRAY_PRIMITIVE_INDEX_H__

namespace pvcop
{

namespace db
{

namespace primitive
{

/**
 * These two overloads return the index of a current iterator no matter
 * what its real type is (iterator or sel_iterator)
 */

template <typename T>
static index_t index(const T&, index_t& index)
{
	return index;
}

template <typename T>
// static index_t index(const typename pvcop::core::selected_array<T>::const_iterator& it, index_t&)
// // overload resolution fails with this alias
// static index_t index(typename pvcop::core::__impl::type_traits<T>::const_sel_iterator& it,
// index_t&) // overload resolution fails with this alias
static index_t index(const typename pvcop::core::__impl::sel_iterator_impl<T, true>& it,
                     index_t&) // overload resolution works both with GCC and clang
{
	return it.index();
}

} // namespace primitive

} // namespace db

} // namespace pvcop

#endif // __PVCOP_DB_ARRAY_PRIMITIVE_INDEX_H__
