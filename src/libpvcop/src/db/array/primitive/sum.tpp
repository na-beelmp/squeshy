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

#ifndef __DB_ARRAY_PRIMITIVE_SUM_TPP__
#define __DB_ARRAY_PRIMITIVE_SUM_TPP__

#include "compensated_summation.h"
#include <pvcop/db/exceptions/operation_not_supported.h>
#include <boost/type_traits/has_plus.hpp>

namespace pvcop
{

namespace db
{

namespace primitive
{

namespace __impl
{

template <class Array, class T>
static void sum(Array const& array, T& res)
{
	res = std::accumulate(array.begin(), array.end(), T());
}

/**
 * Specialisation to use a compensated summation for floating point types
 */
template <class Array>
static void sum(Array const& array, double& res)
{
	compensated_summation_t init;

	const compensated_summation_t& result = std::accumulate(
	    array.begin(), array.end(), init, compensated_summation<typename Array::value_type>);

	res = result.sum;
}

} // pvcop::db::primitive::__impl

template <class Array>
static core::memarray<typename sum_type<typename Array::value_type>::type> sum(Array const& array)
{
	if
		constexpr(boost::has_plus_assign<typename Array::value_type>::value)
		{
			typename sum_type<typename Array::value_type>::type res;

			__impl::sum(array, res);

			core::memarray<typename sum_type<typename Array::value_type>::type> elt_array(
			    array.size() ? 1 : 0);
			if (elt_array.size() == 1) {
				elt_array[0] = res;
			}

			return elt_array;
		}

	throw db::exception::operation_not_supported(
	    "Array underlying type does not support plus operator");
}

} // pvcop::db::primitive

} // pvcop::db

} // pvcop

#endif // __DB_ARRAY_PRIMITIVE_SUM_TPP__
