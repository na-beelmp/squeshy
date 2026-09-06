#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_COUNT_TPP
#define PVCOP_SRC_ARRAY_PRIMITIVE_GROUP_COUNT_TPP

namespace pvcop
{
namespace db
{
namespace primitive
{

/******************************************************************************
 *
 * group_count
 *
 ******************************************************************************/

template <class T>
static core::memarray<T> group_count(core::array<index_t> const& groups,
                                     size_t extents_size // Number of different values
                                     )
{

	pvlogger::trace() << "db::primitive::group_count(...)" << std::endl;

	// Create corresponding arrays
	core::memarray<T> count_array(extents_size, true);

	for (index_t grp : groups) {
		++count_array[grp];
	}

	return count_array;
}
}
}
}

#endif
