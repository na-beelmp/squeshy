/* * MIT License
 *
 * © ESI Group, 2015
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __DB_ARRAY_PRIMITIVE_SUM_H__
#define __DB_ARRAY_PRIMITIVE_SUM_H__

#include <pvcop/db/exceptions/operation_not_supported.h>
#include <boost/type_traits/has_plus_assign.hpp>

namespace pvcop
{

namespace db
{

namespace primitive
{

namespace __impl
{

template <typename T1,
          typename T2,
          class = typename std::enable_if<boost::has_plus_assign<T1>::value>::type>
void increment(T1& t1, T2 t2)
{
	t1 += t2;
}

template <typename T1,
          typename T2,
          class = typename std::enable_if<not boost::has_plus_assign<T1>::value>::type>
void increment(T1&, const T2&)
{
	throw db::exception::operation_not_supported(
	    "Array underlying type does not support plus operator");
}

} // namespace __impl

template <typename T>
struct sum_type {
  private:
	using integer_type =
	    typename std::conditional<std::is_signed<T>::value, int64_t, uint64_t>::type;
	using promoted_pod_type =
	    typename std::conditional<std::is_floating_point<T>::value or
	                                  std::is_same<T, pvcop::db::uint128_t>::value,
	                              double,
	                              integer_type>::type;

  public:
	using type = typename std::conditional<std::is_standard_layout<T>::value and std::is_trivial<T>::value, promoted_pod_type, T>::type;
};

template <class Array>
static core::memarray<typename sum_type<typename Array::value_type>::type> sum(Array const& array);

} // pvcop::db::primitive

} // pvcop::db

} // pvcop

#endif // __DB_ARRAY_PRIMITIVE_SUM_H__
