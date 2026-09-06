#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_MAX_TPP
#define PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_MAX_TPP

namespace pvcop
{
namespace db
{
namespace primitive
{

/******************************************************************************
 *
 * group_max
 *
 ******************************************************************************/

template <class Array>
static std::tuple<core::memarray<typename Array::value_type>, core::memarray<bool>>
group_max(Array const& array,
          core::array<index_t> const& groups,
          size_t extents_size,
          const core::selection& invalid_sel)
{

	pvlogger::trace() << "db::primitive::group_max(...)" << std::endl;

	using type_t = typename Array::value_type;

	// Create corresponding arrays
	core::memarray<type_t> max_array(extents_size);
	core::memarray<bool> valid_values_sel(extents_size, true);

	std::fill(max_array.begin(), max_array.end(),
	          std::numeric_limits<typename Array::value_type>::lowest());

	index_t idx = 0;
	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, idx++) {
		bool is_invalid = invalid_sel and invalid_sel[value_it.index()];
		if (not is_invalid) {
			max_array[groups[idx]] = std::max<type_t>(max_array[groups[idx]], *value_it);
			valid_values_sel[groups[idx]] = true;
		}
	}

	valid_values_sel = ~valid_values_sel;

	return std::tuple<core::memarray<typename Array::value_type>, core::memarray<bool>>(
	    std::move(max_array),
	    extents_size > 0 and pvcop::core::algo::bit_count(valid_values_sel) > 0
	        ? std::move(valid_values_sel)
	        : core::memarray<bool>());
}
}
}
}

#endif
