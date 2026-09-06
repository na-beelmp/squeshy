#ifndef __PVCOP_SRC_ARRAY_PRIMITIVE_SUBSELECT_TPP__
#define __PVCOP_SRC_ARRAY_PRIMITIVE_SUBSELECT_TPP__

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
 * subselect
 *
 ******************************************************************************/

template <class Array>
static void subselect(Array const& array, Array const& values, db::selection& output_sel)
{
	pvlogger::trace() << "db::primitive::subselect(...)" << std::endl;

	if (not values) {
		return;
	}

	using type_t = typename Array::value_type;

	// special case if the search list contains only one element
	if (values.size() == 1) {
		const auto value = *values.begin();
		index_t idx = 0;
		for (auto it = array.begin(); it != array.end(); ++it, idx++) {
			if (*it == value) {
				output_sel[index(it, idx)] = true;
			}
		}
		return;
	}

	/**
	 * construct set for quicker lookup
	 * @note when the set goes very big its deallocation can be more costly than the whole
	 * computation !
	 */
	std::unordered_set<type_t, std::hash<type_t>, std::equal_to<type_t>,
	                   tbb::scalable_allocator<type_t>>
	    set;
	set.reserve(values.size());
	for (auto v : values) {
		set.insert(v);
	}

	index_t idx = 0;
	for (auto it = array.begin(); it != array.end(); ++it, idx++) {
		if (set.find(*it) != set.end()) {
			output_sel[index(it, idx)] = true;
		}
	}
}

} // pvcop

} // pvcop::db

} // pvcop::db::primitive

#endif // __PVCOP_SRC_ARRAY_PRIMITIVE_SUBSELECT_TPP__
