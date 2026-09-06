//
// MIT License
//
// © ESI Group, 2015
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef __ARRAY_PRIMITIVE_MIN_MAX_TPP__
#define __ARRAY_PRIMITIVE_MIN_MAX_TPP__

namespace pvcop
{

namespace db
{

namespace primitive
{

namespace __impl
{

/**
 * Return the proper memarray with one element or an invalid memarray
 */
template <class Array, class It>
static core::memarray<typename Array::value_type> safe_elt(Array const& array, const It& elt_it)
{
	if (elt_it != array.end()) {

		core::memarray<typename Array::value_type> elt_array(1);
		elt_array[0] = *elt_it;

		return elt_array;
	}

	return {};
}

} // pvcop::db::primitive::__impl

template <class Array>
static core::memarray<typename Array::value_type> min(Array const& array)
{
	const auto& elt_it = std::min_element(array.begin(), array.end());
	return __impl::safe_elt(array, elt_it);
}

template <class Array>
static core::memarray<typename Array::value_type> max(Array const& array)
{
	const auto& elt_it = std::max_element(array.begin(), array.end());
	return __impl::safe_elt(array, elt_it);
}

template <class Array>
static core::memarray<typename Array::value_type> minmax(Array const& array)
{
	typename Array::value_type min = {};
	typename Array::value_type max = {};

	bool min_set = false;
	bool max_set = false;

	for (auto elts : array) {
		if (not max_set or elts > max) {
			max = elts;
			max_set = true;
		}
		if (not min_set or elts < min) {
			min = elts;
			min_set = true;
		}
	}

	core::memarray<typename Array::value_type> elt_array(min_set or max_set ? 2 : 0);
	if (min_set and max_set) {
		elt_array[0] = min;
		elt_array[1] = max;
	} else if (min_set != max_set) {
		elt_array[0] = min_set ? min : max;
		elt_array[1] = elt_array[0];
	}

	return elt_array;
}

} // pvcop::db::primitive

} // pvcop::db

} // pvcop

#endif // __ARRAY_PRIMITIVE_MIN_MAX_TPP__
