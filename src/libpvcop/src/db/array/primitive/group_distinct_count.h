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

#ifndef __PVCOP_ARRAY_PRIMITIVE_GROUP_DISTINCT_COUNT_H__
#define __PVCOP_ARRAY_PRIMITIVE_GROUP_DISTINCT_COUNT_H__

namespace pvcop
{

namespace db
{

namespace primitive
{

template <class Array>
static core::memarray<index_t> group_distinct_count(Array const& array,
                                                    core::array<index_t> const& groups,
                                                    core::array<index_t> const& extents);

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_ARRAY_PRIMITIVE_GROUP_DISTINCT_COUNT_H__
