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

#ifndef __PVCOP_CORE_IMPL_MAP_REDUCE_H__
#define __PVCOP_CORE_IMPL_MAP_REDUCE_H__

#include <pvcop/core/impl/bit.h>

#include <pvhwloc.h>
#include <pvlogger.h>

#include <tbb/parallel_for.h>

#include <cmath>

namespace pvcop
{
namespace core
{
namespace algo
{
namespace __impl
{

#define SEQUENTIAL 0

/**
 * Compute the best slicing ranges (with roughly the same amount of selected
 * values in each slice) in order to make optimal parallel operations.
 *
 * note : slices ranges must be multiple of `pvcop::core::__impl::bit::chunk_bit_size`
 * in order to correctly handle bool support (used when slicing selection)
 */
using slices_ranges_t = std::vector<std::pair<size_t, size_t>>;
static slices_ranges_t slices_ranges(const db::selection& sel, const size_t array_size)
{
	const size_t threads_count = pvhwloc::core_count();
	static constexpr size_t min_slice_count = pvcop::core::__impl::bit_manip::chunk_bit_size;

	slices_ranges_t slices_ranges;

	if (sel) {
		const size_t bit_per_slice = core::algo::bit_count(sel) / threads_count;

		if (bit_per_slice == 0) {
			slices_ranges.emplace_back(0, array_size);
			return slices_ranges;
		}

		size_t last_pos = 0;
		for (size_t th_index = 0; th_index < threads_count; th_index++) {

			size_t nth_set_bit = find_nth_set_bit(sel.data(), bit_per_slice, last_pos, sel.size());

			const size_t range = (nth_set_bit - last_pos) & ~(min_slice_count - 1);

			const size_t pos = last_pos;
			const size_t len = th_index == (threads_count - 1) ? array_size - pos : range;

			if (len) {
				slices_ranges.emplace_back(pos, len);
			}

			last_pos = pos + len;
		}
	} else {
		const size_t slices_count =
		    std::min(threads_count, (size_t)std::ceil((double)array_size / min_slice_count));

		for (size_t slice_index = 0; slice_index < slices_count; slice_index++) {
			const size_t range =
			    std::max(array_size / threads_count, min_slice_count) & ~(min_slice_count - 1);

			const size_t pos = slice_index * range;
			const size_t len = slice_index == (slices_count - 1) ? array_size - pos : range;

			slices_ranges.emplace_back(pos, len);
		}
	}

	return slices_ranges;
}

namespace
{

template <typename Fm>
void do_map(const slices_ranges_t& sr, Fm&& map_func)
{
#if SEQUENTIAL
	for (size_t slice_index = 0; slice_index < sr.size(); slice_index++) {
#else
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, sr.size(), 1),
		[&](tbb::blocked_range<size_t> const& th) {
		const size_t slice_index = th.begin();
#endif
		const size_t pos = sr[slice_index].first;
		const size_t len = sr[slice_index].second;

		map_func(pos, len, slice_index);
	}
#if not SEQUENTIAL
	, tbb::simple_partitioner());
#endif
}
}

/**
 * Helper function aimed at realising parallel mapping operations easily.
 *
 * This is the only function intended to use tbb::parallel_for directly.
 *
 * core::algo::__impl::map(array.size(),
 *     [&](size_t pos, size_t len, size_t th_index) {
 *      // @pos = starting index of the range
 *      // @len = length of the range
 *      // @th_index= index of the underlying thread
 *
 *		// code goes here...
 *	});
 */
template <typename Fi, typename Fm>
void map(const db::selection& sel, const size_t array_size, Fi&& init_func, Fm&& map_func)
{
	const slices_ranges_t& sr = slices_ranges(sel, array_size);

	init_func(sr.size());
	do_map(sr, map_func);
}

template <typename Fm>
void map(const db::selection& sel, const size_t array_size, Fm&& map_func)
{
	const slices_ranges_t& sr = slices_ranges(sel, array_size);

	do_map(sr, map_func);
}

/**
 * map_reduce function calling a reduce function after the mapping function
 */
template <typename Fi, typename Fm, typename Fr>
void map_reduce(
    const db::selection& sel, const size_t size, Fi&& init_func, Fm&& map_func, Fr&& reduce_func)
{
	const slices_ranges_t& sr = slices_ranges(sel, size);

	init_func(sr.size());
	do_map(sr, map_func);
	reduce_func();
}

template <typename Fm, typename Fr>
void map_reduce(const db::selection& sel, const size_t size, Fm&& map_func, Fr&& reduce_func)
{
	map(sel, size, map_func);
	reduce_func();
}
}
}
}
} // namespace pvcop::core::algo::__impl

#endif // __PVCOP_CORE_IMPL_MAP_REDUCE_H__
