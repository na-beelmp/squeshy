#ifndef __PVCOP_SRC_ARRAY_PRIMITIVE_EQUALRANGE_TPP__
#define __PVCOP_SRC_ARRAY_PRIMITIVE_EQUALRANGE_TPP__

#include <unordered_set>
#include <tbb/scalable_allocator.h>

namespace pvcop
{

namespace db
{

namespace primitive
{

/******************************************************************************
 *
 * equal_range
 *
 ******************************************************************************/

template <class Array, typename T>
static db::range_t equal_range(Array const& array,
                               core::array<bool> const& invalid_sel,
                               core::array<T> const& minmax,
                               core::array<index_t> const* sort_order)
{
	pvlogger::trace() << "db::primitive::equal_range(...)" << std::endl;

	if (sort_order) {

		auto less_f = [&](index_t idx, T value) {
			return value > array[idx] and (not invalid_sel or not invalid_sel[idx]);
		};
		auto greater_f = [&](T value, index_t idx) {
			return value < array[idx] or (invalid_sel and invalid_sel[idx]);
		};
		auto lower = std::lower_bound(sort_order->begin(), sort_order->end(), minmax[0], less_f);
		if (lower == sort_order->end()) {
			lower = sort_order->begin();
		}
		auto upper = std::upper_bound(lower, sort_order->end(), minmax[1], greater_f);

		return db::range_t{(size_t)std::distance(sort_order->begin(), lower),
		                   (size_t)std::distance(sort_order->begin(), upper)};
	} else {
		auto lower = std::lower_bound(array.begin(), array.end(), minmax[0]);
		if (lower == array.end()) {
			lower = array.begin();
		}
		auto upper = std::upper_bound(lower, array.end(), minmax[1]);

		return db::range_t{(size_t)std::distance(array.begin(), lower),
		                   (size_t)std::distance(array.begin(), upper)};
	}
}

} // pvcop

} // pvcop::db

} // pvcop::db::primitive

#endif // __PVCOP_SRC_ARRAY_PRIMITIVE_EQUALRANGE_TPP__
