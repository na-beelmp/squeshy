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

#ifndef PVCOP_DB_ARRAY_SELECTED_ARRAY_H
#define PVCOP_DB_ARRAY_SELECTED_ARRAY_H

#include <pvcop/core/array.h>
#include <pvcop/db/types.h>

namespace pvcop
{

namespace core
{

/**
 * Selected array is a core::array with a selection applied on it.
 *
 * It doesn't have ownership of its data. User have to make sure encapsulated data
 * are available when they use it.
 *
 * It provides an iterator just likes others with begin/end construct.
 */
template <class T>
class selected_array
{
  public:
	using __type_traits = core::__impl::type_traits<T>;
	using iterator = typename __type_traits::sel_iterator;
	using const_iterator = typename __type_traits::const_sel_iterator;
	using value_type = T;

  public:
	selected_array(core::array<T> const& data, core::array<bool> const& sel)
	    : _data(data), _sel(sel)
	{
		assert(not _sel or _data.size() == _sel.size());
	}

	/**
	 * @return a const_sel_iterator referring to the first selected value
	 */
	inline const_iterator begin() const
	{
		return __type_traits::get_const_sel_iterator(_data.data(), _sel.data(), 0, _data.size());
	}
	inline const_iterator cbegin() const { return begin(); }

	/**
	 * @return a const_sel_iterator referring to past-the-end element
	 */
	inline const_iterator end() const
	{
		return __type_traits::get_const_sel_iterator(_data.data(), _sel.data(), _data.size(),
		                                             _data.size());
	}
	inline const_iterator cend() const { return end(); }

	inline operator bool() const { return _data; }

	size_t size() const
	{
		// TODO : bench this to see if worth caching
		return _sel ? core::algo::bit_count(_sel) : _data.size();
	}

  private:
	core::array<T> const& _data;   //!< Underlying array
	core::array<bool> const& _sel; //!< Used selection to filter array
};

/**
 * Helper to create selected_array without specifiying type information.
 */
template <class T>
selected_array<T> make_selected_array(core::array<T> const& data, core::array<bool> const& sel)
{
	return selected_array<T>(std::forward<core::array<T> const&>(data),
	                         std::forward<core::array<bool> const&>(sel));
}

} // namespace pvcop::core

} // namespace pvcop

#endif
