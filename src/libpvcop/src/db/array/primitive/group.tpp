#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_TPP
#define PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_TPP

namespace pvcop
{

namespace db
{

namespace primitive
{

/******************************************************************************
 *
 * group
 *
 ******************************************************************************/

template <class Array>
group_results_t group(Array const& array, const core::selection& invalid_selection)
{
	pvlogger::trace() << "db::primitive::group(...)" << std::endl;

	// Create corresponding arrays
	core::memarray<index_t> groups_array(array.size());
	core::memarray<index_t> extents_array(array.size());
	core::memarray<bool> invalid_groups_sel(array.size());

	// mapping from value to group_id
	std::unordered_map<typename Array::value_type, index_t> valid_mapping;
	std::unordered_map<typename Array::value_type, index_t> invalid_mapping;
	index_t row = 0;               // handled line
	index_t valid_num_group = 0;   // group number for valid values
	index_t invalid_num_group = 0; // group number for invalid values

	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, row++) {
		const index_t idx = index(value_it, row);

		bool invalid = invalid_selection ? invalid_selection[idx] : false;
		std::unordered_map<typename Array::value_type, index_t>& mapping =
		    invalid ? invalid_mapping : valid_mapping;
		index_t& num_group = invalid ? invalid_num_group : valid_num_group;
		index_t num_group_index =
		    invalid ? groups_array.size() - invalid_num_group - 1 : valid_num_group;

		auto it = mapping.find(*value_it);
		if (it == mapping.end()) {
			// If value was not met before, add it to the mapping and fill arrays
			mapping.emplace(std::pair<typename Array::value_type, index_t>(*value_it, num_group));

			groups_array[row] = num_group;
			extents_array[num_group_index] = idx;
			invalid_groups_sel[row] = invalid;

			num_group++;
		} else {
			// Group already exists so we just have to set group_id
			groups_array[row] = it->second;
			invalid_groups_sel[row] = invalid;
		}
	}

	// increase invalid groups values
	const auto& invalid_groups_array = core::make_selected_array(groups_array, invalid_groups_sel);
	for (auto it = invalid_groups_array.begin(); it < invalid_groups_array.end(); ++it) {
		groups_array[it.index()] += valid_num_group;
	}

	// reorder extents values
	const size_t total_group_count = valid_num_group + invalid_num_group;
	for (size_t i = 0;
	     i < std::min((size_t)invalid_num_group,
	                  ((extents_array.size() - total_group_count + invalid_num_group) / 2));
	     i++) {
		const size_t old_index = extents_array.size() - i - 1;
		const size_t new_index = valid_num_group + i;
		std::swap(extents_array[new_index], extents_array[old_index]);
	}

	// set invalid extents selection
	core::memarray<bool> invalid_extents_sel;
	if (invalid_num_group) {
		invalid_extents_sel = core::memarray<bool>(total_group_count);
		std::fill(invalid_extents_sel.begin(), invalid_extents_sel.begin() + valid_num_group + 1,
		          false);
		std::fill(invalid_extents_sel.begin() + valid_num_group, invalid_extents_sel.end(), true);
	} else {
		invalid_groups_sel = pvcop::core::memarray<bool>();
	}

	// shrink to fit real size
	extents_array.resize(total_group_count);

	return group_results_t{std::move(groups_array), std::move(invalid_groups_sel),
	                       std::move(extents_array), std::move(invalid_extents_sel)};
}

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif
