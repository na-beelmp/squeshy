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

#ifndef PVCOP_SRC_ARRAY_PRIMITIVE_DIVIDE_H
#define PVCOP_SRC_ARRAY_PRIMITIVE_DIVIDE_H

#include <pvcop/db/exceptions/operation_not_supported.h>

#include <boost/type_traits/has_divides.hpp>

namespace pvcop
{

namespace db
{

namespace primitive
{

template <class T1, class T2>
static core::memarray<typename div_type<T1>::type> divide(core::array<T1> const& array,
                                                          core::array<T2> const& divisors,
                                                          const core::selection& selection)
{
	// if constexpr (has_operator_div<T1, T2>::value) {
	if
		constexpr(boost::has_divides<T1, T2>::value)
		{
			size_t sel_count = selection ? core::algo::bit_count(selection) : array.size();

			// Create corresponding arrays
			core::memarray<typename div_type<T1>::type> divided_array(sel_count);

			size_t j = 0;
			for (size_t i = 0; i < array.size(); i++) {
				if (not selection or (selection and selection[i])) {
					divided_array[j] = (typename div_type<T1>::type)array[i] / divisors[i];
					j++;
				}
			}

			return divided_array;
		}

	throw db::exception::operation_not_supported(
	    "Array underlying type does not support division operator");
}

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif // PVCOP_SRC_ARRAY_PRIMITIVE_DIVIDE_H
