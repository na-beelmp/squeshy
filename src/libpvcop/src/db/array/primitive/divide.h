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

#ifndef PVCOP_DB_ARRAY_PRIMITIVE_DIVIDE_H
#define PVCOP_DB_ARRAY_PRIMITIVE_DIVIDE_H

namespace pvcop
{

namespace db
{

namespace primitive
{

template <typename T>
struct div_type {
	using type = typename std::conditional<std::is_standard_layout<T>::value and std::is_trivial<T>::value, double, T>::type;
};

/**
 * Divide an array by another one of the same size
 * Each values are divided by the corresponding divisor
 *
 * @see core::array.divide
 */
template <class T1, class T2>
static core::memarray<typename div_type<T1>::type> divide(core::array<T1> const& array,
                                                          core::array<T2> const& divisors,
                                                          const core::selection& selection);

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif // PVCOP_DB_ARRAY_PRIMITIVE_DIVIDE_H //
