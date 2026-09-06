#ifndef __PVCOP_SRC_ARRAY_PRIMITIVE_RANGE_SELECT_TPP__
#define __PVCOP_SRC_ARRAY_PRIMITIVE_RANGE_SELECT_TPP__

namespace pvcop
{

namespace db
{

namespace primitive
{

/******************************************************************************
 *
 * range_select
 *
 ******************************************************************************/

template <class Array>
static void range_select(Array const& array,
                         typename Array::value_type min,
                         typename Array::value_type max,
                         db::selection& output_sel)
{
	pvlogger::trace() << "db::primitive::range_select(...)" << std::endl;

	std::fill(output_sel.begin(), output_sel.end(), false);

	index_t idx = 0;
	for (auto it = array.begin(); it != array.end(); ++it, idx++) {
		if (min <= *it and max >= *it) {
			output_sel[index(it, idx)] = true;
		}
	}
}

} // pvcop

} // pvcop::db

} // pvcop::db::primitive

#endif // __PVCOP_SRC_ARRAY_PRIMITIVE_RANGE_SELECT_TPP__
