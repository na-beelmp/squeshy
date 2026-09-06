#ifndef __PVCOP_SRC_ARRAY_PRIMITIVE_SUBTRACT_TPP__
#define __PVCOP_SRC_ARRAY_PRIMITIVE_SUBTRACT_TPP__

namespace pvcop
{

namespace db
{

namespace primitive
{

/******************************************************************************
 *
 * subtract
 *
 ******************************************************************************/

template <class SubType, class T, class I, class F>
core::memarray<SubType> subtract(core::array<T> const& array,
                                 core::array<T> const& values,
                                 core::array<I> const& groups,
                                 F f)
{
	core::memarray<SubType> subtracted_array(array.size(), /* init */ false);

#pragma omp parallel for
	for (size_t i = 0; i < array.size(); i++) {
		subtracted_array[i] = f(array[i] - values[groups[i]]);
	}

	return subtracted_array;
}

template <class SubType, class T, class I>
core::memarray<SubType>
subtract(core::array<T> const& array, core::array<T> const& values, core::array<I> const& groups)
{
	core::memarray<SubType> subtracted_array(array.size(), /* init */ false);

#pragma omp parallel for
	for (size_t i = 0; i < array.size(); i++) {
		subtracted_array[i] = array[i] - values[groups[i]];
	}

	return subtracted_array;
}

} // namespace primitive

} // namespace db

} // namespace pvcop

#endif // __PVCOP_SRC_ARRAY_PRIMITIVE_SUBTRACT_TPP__
