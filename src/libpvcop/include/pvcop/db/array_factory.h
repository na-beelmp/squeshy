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

#ifndef __PVCOP_DB___IMPL_ARRAY_FACTORY_H__
#define __PVCOP_DB___IMPL_ARRAY_FACTORY_H__

#include <pvcop/db/array_factory_interface.h>
#include <db/array/array_impl.h>

namespace pvcop
{

namespace core
{

template <typename T>
class array;

template <typename T>
class memarray;

} // namespace pvcop::core

namespace db
{

/**
 * This class is intended to create db::array_impl instances for a type T.
 *
 * This class instances are create in db::types_manager::register_type()
 */
template <typename T>
class array_factory : public array_factory_interface
{
  public:
	using formatter_sp = array_factory_interface::formatter_sp;

  public:
	/**
	 * Default constructor
	 */
	array_factory(type_t type) : array_factory_interface(type) {}

  public:
	/**
	 * Create a new db::array_impl without ownership by slicing an existing db::array_impl
	 *
	 * @param pimpl the original array pimpl
	 * @param pos the start position in @a array
	 * @param len the size of the new array
	 *
	 * @return the proper array implementation interface
	 */
	array_impl_interface*
	create_array(const array_impl_interface* pimpl, size_t pos, size_t len) const override
	{
		return new array_impl<core::array, T>(pimpl, pos, len);
	}

	/**
	 * Create a brand new db::array_impl with memory ownership
	 *
	 * @param count the number of elements contained in the array
	 * @param formatter the proper formatter to convert data to string (optional)
	 *
	 * @return the proper array implementation interface
	 */
	array_impl_interface*
	create_memarray(size_t count, const formatter_sp& formatter, bool init = false) const override
	{
		return new array_impl<core::memarray, T>(count, formatter, nullptr, nullptr, {}, init);
	}

	/**
	 * Create a brand new db::array bound to a mapped file
	 *
	 * @param data the base address of the file mapped array
	 * @param count the number of elements contained in the array
	 * @param rf the release function to use when the array will be freed
	 * @param dict the dictionary corresponding to this array
	 * @param formatter the proper formatter to convert data to string (optional)
	 *
	 * @return the proper array implementation interface
	 */
	array_impl_interface*
	create_filearray(void* data,
	                 size_t count,
	                 const core::filearray_release_f& rf,
	                 const read_dict* dict,
	                 const formatter_sp formatter = formatter_sp(),
	                 void* invalid_sel_data = nullptr,
	                 const core::filearray_release_f& invalid_values_rf = []() {},
	                 const read_dict* invalid_values_dict = nullptr) const override
	{
		return new array_impl<core::filearray, T>(
		    data, count, formatter, rf, dict,
		    core::filearray<bool>(
		        ((typename core::__impl::type_traits<bool>::value_type*)invalid_sel_data), count,
		        invalid_values_rf),
		    invalid_values_dict);
	}
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB___IMPL_ARRAY_FACTORY_H__
