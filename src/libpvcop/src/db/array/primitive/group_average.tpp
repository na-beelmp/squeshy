#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_AVERAGE_TPP
#define PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_AVERAGE_TPP

namespace pvcop
{

namespace db
{

namespace primitive
{

#include "sum.h"
#include "compensated_summation.h"
#include <pvcop/db/exceptions/operation_not_supported.h>

#include <boost/type_traits/has_divides.hpp>

/******************************************************************************
 *
 * group_average
 *
 ******************************************************************************/

namespace __impl
{

template <class Array,
          class = typename std::enable_if<
              not std::is_same<typename Array::value_type, double>::value>::type>
static core::memarray<typename div_type<typename Array::value_type>::type>
group_average(Array const& array,
              core::array<index_t> const& groups,
              const size_t extents_size,
              const core::selection& invalid_sel)
{
	pvlogger::trace() << "db::primitive::group_average(...)" << std::endl;

	// Create corresponding arrays
	core::memarray<typename sum_type<typename Array::value_type>::type> sum_array(extents_size,
	                                                                              true);
	core::memarray<typename div_type<typename Array::value_type>::type> average_array(extents_size,
	                                                                                  true);
	core::memarray<index_t> histo_array(extents_size, true);

	index_t idx = 0;
	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, idx++) {
		bool is_invalid = invalid_sel and invalid_sel[value_it.index()];
		if (not is_invalid) {
			increment(sum_array[groups[idx]], *value_it);
			++histo_array[groups[idx]];
		}
	}

	// Divide by the number of values
	for (size_t i = 0; i < average_array.size(); i++) {
		average_array[i] =
		    (typename div_type<typename Array::value_type>::type)sum_array[i] / histo_array[i];
	}

	return average_array;
}

template <
    class Array,
    class = typename std::enable_if<std::is_same<typename Array::value_type, double>::value>::type>
static core::memarray<double> group_average(Array const& array,
                                            core::array<index_t> const& groups,
                                            const size_t extents_size,
                                            const core::selection& invalid_sel)
{
	pvlogger::trace() << "db::primitive::group_average(...)" << std::endl;

	// Create corresponding arrays
	core::memarray<double> average_array(extents_size, true);
	core::memarray<index_t> histo_array(extents_size, true);
	std::vector<__impl::compensated_summation_t> compensated_sum_array(extents_size);

	index_t idx = 0;
	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, idx++) {
		bool is_invalid = invalid_sel and invalid_sel[value_it.index()];
		if (not is_invalid) {
			__impl::compensated_summation_t& sum = compensated_sum_array[groups[idx]];
			sum = __impl::compensated_summation(sum, *value_it);
			++histo_array[groups[idx]];
		}
	}

	// Divide by the number of values
	for (size_t i = 0; i < average_array.size(); i++) {
		average_array[i] = compensated_sum_array[i].sum / histo_array[i];
	}

	return average_array;
}

} // namespace __impl

template <class Array>
static core::memarray<typename div_type<typename Array::value_type>::type>
group_average(Array const& array,
              core::array<index_t> const& groups,
              const size_t extents_size,
              const core::selection& invalid_sel)
{
	if
		constexpr(boost::has_divides<typename Array::value_type, index_t>::value)
		{
			return __impl::group_average(array, groups, extents_size, invalid_sel);
		}

	(void)groups;
	(void)extents_size;
	(void)invalid_sel;

	throw db::exception::operation_not_supported(
	    "Array underlying type does not support division operator");
}

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif
