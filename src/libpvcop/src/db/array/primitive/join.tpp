#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_JOIN_TPP
#define PVCOP_SRC_ARRAY_PRIMITIVE_JOIN_TPP

#include <pvcop/core/algo/selection.h>

namespace pvcop
{
namespace db
{
namespace primitive
{

/******************************************************************************
 *
 * join
 *
 ******************************************************************************/

template <class Array>
static core::memarray<typename Array::value_type> join(Array const& array,
                                                       core::array<index_t> const& indices)
{
	pvlogger::trace() << "db::primitive::join(...)" << std::endl;

	// Create corresponding arrays
	core::memarray<typename Array::value_type> joined_array(indices.size());

	std::transform(indices.begin(), indices.end(), joined_array.begin(),
	               [&array](index_t i) { return array[i]; });

	return joined_array;
}

template <class Array>
static std::tuple<core::memarray<typename Array::value_type>, core::memarray<bool>>
join(Array const& array, core::array<bool> const& in_sel, const core::selection& invalid_selection)
{
	pvlogger::trace() << "db::primitive::join(...)" << std::endl;

	// Create corresponding arrays
	size_t bit_count = pvcop::core::algo::bit_count(in_sel);
	core::memarray<typename Array::value_type> joined_array(bit_count);
	core::memarray<bool> joined_invalid_sel;

	if (invalid_selection and pvcop::core::algo::bit_count(invalid_selection) > 0) {
		joined_invalid_sel = core::memarray<bool>(joined_array.size(), true);
	}

	size_t index = 0;
	const auto& sel_array = make_selected_array(array, in_sel);
	for (auto it = sel_array.begin(); it != sel_array.end(); ++it, index++) {
		joined_array[index] = *it;
		if (invalid_selection and invalid_selection[it.index()]) {
			joined_invalid_sel[index] = true;
		}
	}

	return std::tuple<core::memarray<typename Array::value_type>, core::memarray<bool>>(
	    std::move(joined_array), std::move(joined_invalid_sel));
}

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif
