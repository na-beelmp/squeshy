#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_MIN_TPP
#define PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_MIN_TPP

namespace pvcop
{

namespace db
{

namespace primitive
{

/******************************************************************************
 *
 * group_min
 *
 ******************************************************************************/

template <class Array>
static std::tuple<core::memarray<typename Array::value_type>, core::memarray<bool>>
group_min(Array const& array,
          core::array<index_t> const& groups,
          size_t extents_size,
          const core::selection& invalid_sel)
{

	pvlogger::trace() << "db::primitive::group_min(...)" << std::endl;

	using type_t = typename Array::value_type;

	// Create corresponding arrays
	core::memarray<type_t> min_array(extents_size);
	core::memarray<bool> valid_values_sel(extents_size, true);

	std::fill(min_array.begin(), min_array.end(), std::numeric_limits<type_t>::max());

	index_t idx = 0;
	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, idx++) {
		bool is_invalid = invalid_sel and invalid_sel[value_it.index()];
		if (not is_invalid) {
			min_array[groups[idx]] = std::min<type_t>(min_array[groups[idx]], *value_it);
			valid_values_sel[groups[idx]] = true;
		}
	}

	valid_values_sel = ~valid_values_sel;

	return std::tuple<core::memarray<typename Array::value_type>, core::memarray<bool>>(
	    std::move(min_array),
	    extents_size > 0 and pvcop::core::algo::bit_count(valid_values_sel) > 0
	        ? std::move(valid_values_sel)
	        : core::memarray<bool>());
}

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif
