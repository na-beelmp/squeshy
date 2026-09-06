#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_SUM_TPP
#define PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_SUM_TPP

#include "compensated_summation.h"
#include <pvcop/core/impl/type_traits.h>

namespace pvcop
{

namespace db
{

namespace primitive
{

namespace __impl
{

template <class Array, class SumArray>
static void group_sum(Array const& array,
                      SumArray& sum_array,
                      core::array<index_t> const& groups,
                      size_t /*extents_size*/,
                      const core::selection& invalid_sel)
{
	index_t idx = 0;
	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, idx++) {
		bool is_invalid = invalid_sel and invalid_sel[value_it.index()];
		if (not is_invalid) {
			increment(sum_array[groups[idx]],
			          (typename sum_type<typename Array::value_type>::type) * value_it);
		}
	}
}

/**
 * Specialisation to use a compensated summation for floating point types
 */
template <class Array>
static void group_sum(Array const& array,
                      core::array<double>& sum_array,
                      core::array<index_t> const& groups,
                      size_t extents_size,
                      const core::selection& invalid_sel)
{
	std::vector<__impl::compensated_summation_t> compensated_sum_array(extents_size);

	index_t idx = 0;
	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, idx++) {
		bool is_invalid = invalid_sel and invalid_sel[value_it.index()];
		if (not is_invalid) {
			__impl::compensated_summation_t& sum = compensated_sum_array[groups[idx]];
			sum = __impl::compensated_summation(sum, *value_it);
		}
	}
	for (size_t i = 0; i < extents_size; i++) {
		sum_array[i] = compensated_sum_array[i].sum;
	}
}

} // namespace __impl

/******************************************************************************
 *
 * group_sum
 *
 ******************************************************************************/

template <class Array>
static core::memarray<typename sum_type<typename Array::value_type>::type>
group_sum(Array const& array,
          core::array<index_t> const& groups,
          size_t extents_size,
          const core::selection& invalid_sel)
{

	pvlogger::trace() << "db::primitive::group_sum(...)" << std::endl;
	using return_type = typename sum_type<typename Array::value_type>::type;

	// Create corresponding arrays
	core::memarray<return_type> sum_array(extents_size, true);

	__impl::group_sum(array, sum_array, groups, extents_size, invalid_sel);

	return sum_array;
}

} // namespace primitive

} // namespace db

} // namespace pvcop

#endif
