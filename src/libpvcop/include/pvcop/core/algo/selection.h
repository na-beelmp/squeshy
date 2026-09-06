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

#ifndef __PVCOP_CORE_ALGO_SELECTION_H__
#define __PVCOP_CORE_ALGO_SELECTION_H__

#include <pvcop/core/impl/bit.h>
#include <pvcop/core/selection_type.h>

namespace pvcop
{

namespace core
{

namespace algo
{
/**
 * Returns the index of the next selected bit
 *
 * search in the range [start, end]
 */
size_t find_first_set_bit(const core::selection& selection, const size_t start, const size_t end);
size_t
find_nth_set_bit(const core::selection& selection, int64_t n, const size_t start, const size_t end);
size_t find_nth_last_set_bit(const core::selection& selection,
                             int64_t n,
                             const size_t start,
                             const size_t end);

/**
 * Returns the set bit count of a selection
 *
 * count in the range [start, end]
 */
size_t bit_count(const core::selection& selection, size_t start = 0, size_t end = 0);
size_t bit_count(const core::__impl::bit_manip::value_type* selection, size_t start, size_t end);

namespace __impl
{

size_t find_first_set_bit(const core::__impl::bit_manip::value_type* selection,
                          const size_t start,
                          const size_t end);

size_t find_nth_set_bit(const core::__impl::bit_manip::value_type* selection,
                        int64_t n,
                        const size_t start,
                        const size_t end);

size_t find_nth_last_set_bit(const core::__impl::bit_manip::value_type* selection,
                             int64_t n,
                             const size_t start,
                             const size_t end);

} // namespace pvcop::core::algo::__impl

} // namespace pvcop::core::algo

} // namespace pvcop::core

} // namespace pvcop

#endif // __PVCOP_CORE_ALGO_SELECTION_H__
