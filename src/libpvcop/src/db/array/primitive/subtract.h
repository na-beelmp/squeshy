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

#ifndef PVCOP_DB_ARRAY_PRIMITIVE_SUBTRACT_H
#define PVCOP_DB_ARRAY_PRIMITIVE_SUBTRACT_H

namespace pvcop
{

namespace db
{

namespace primitive
{

/**
 * Subtract a value to each value of the array
 *
 * @param[in] array : array to be subtracted
 * @param[in] value : value to subtract
 */
template <class SubType, class T, class I, class F>
static core::memarray<SubType> subtract(core::array<T> const& array,
                                        core::array<T> const& values,
                                        core::array<I> const& groups,
                                        F f);

template <class SubType, class T, class I>
static core::memarray<SubType>
subtract(core::array<T> const& array, core::array<T> const& values, core::array<I> const& groups);

} // namespace primitive

} // namespace db

} // namespace pvcop

#endif // PVCOP_DB_ARRAY_PRIMITIVE_SUBTRACT_H
