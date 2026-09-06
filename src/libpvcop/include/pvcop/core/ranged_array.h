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

#ifndef PVCOP_DB_ARRAY_RANGED_ARRAY_H
#define PVCOP_DB_ARRAY_RANGED_ARRAY_H

#include <pvcop/core/array.h>
#include <pvcop/core/range.h>
#include <pvcop/db/types.h>

namespace pvcop
{

namespace core
{

/**
 * Ranged array is a core::array with a continuous range applied on it.
 *
 * It doesn't have ownership of its data. User have to make sure encapsulated data
 * are available when they use it.
 *
 * It provides an iterator just likes others with begin/end construct.
 */
template <class T>
class ranged_array
{
  public:
	using __type_traits = core::__impl::type_traits<T>;
	using iterator = typename __type_traits::iterator;
	using const_iterator = typename __type_traits::const_iterator;
	using value_type = T;

  public:
	ranged_array(core::array<T> const& data, core::range_t const& range)
	    : _data(data), _range(range)
	{
	}

	/**
	 * @return a const_sel_iterator referring to the first selected value
	 */
	inline const_iterator begin() const
	{
		return __type_traits::get_const_iterator(_data.data(), _range.begin);
	}
	inline const_iterator cbegin() const { return begin(); }

	/**
	 * @return a const_sel_iterator referring to past-the-end element
	 */
	inline const_iterator end() const
	{
		return __type_traits::get_const_iterator(_data.data(), _range.end);
	}
	inline const_iterator cend() const { return end(); }

	inline operator bool() const { return _data; }

	size_t size() const { return _range.end - _range.begin; }

  private:
	core::array<T> const& _data; //!< Underlying array
	core::range_t _range;        //!< Used range to shorten array
};

/**
 * Helper to create ranged_array without specifiying type information.
 */
template <class T>
ranged_array<T> make_ranged_array(core::array<T> const& data, core::range_t const& range)
{
	return ranged_array<T>(std::forward<core::array<T> const&>(data),
	                       std::forward<core::range_t const&>(range));
}

} // namespace pvcop::core

} // namespace pvcop

#endif
