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

#ifndef __PVCOP_DB_ARRAY_FACTORY_INTERFACE_H__
#define __PVCOP_DB_ARRAY_FACTORY_INTERFACE_H__

#include <pvcop/types/formatter/formatter_interface.h>

#include <pvcop/db/types.h>

#include <pvcop/core/filearray.h>

namespace pvcop
{

namespace db
{

class array_impl_interface;
class read_dict;

/**
 * Interface class to create db::array_impl instances
 *
 * This class is internally used by db::array_base in order to hide the underlying objects creation.
 */
class array_factory_interface
{
  public:
	using formatter_sp = types::formatter_interface::sp;

  public:
	/**
	 * Constructor
	 */
	array_factory_interface(type_t type) : _type(type) {}

	/**
	 * Destructor
	 */
	virtual ~array_factory_interface(){};

	/**
	 * Create a new db::array_impl without ownership by slicing an existing db::array_impl
	 *
	 * @param pimpl the original array pimpl
	 * @param pos the start position in @a array
	 * @param len the size of the new array
	 *
	 * @return the proper array implementation interface
	 */

	virtual array_impl_interface*
	create_array(const array_impl_interface* pimpl, size_t pos, size_t len) const = 0;

	/**
	 * Create a brand new db::array_impl with memory ownership
	 *
	 * @param count the number of elements contained in the array
	 * @param formatter the proper formatter to convert data to string (optional)
	 *
	 * @return the proper array implementation interface
	 */
	virtual array_impl_interface*
	create_memarray(size_t count, const formatter_sp& formatter, bool init = false) const = 0;

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
	virtual array_impl_interface*
	create_filearray(void* data,
	                 size_t count,
	                 const core::filearray_release_f& rf,
	                 const read_dict* dict,
	                 const formatter_sp formatter = formatter_sp(),
	                 void* invalid_sel_data = nullptr,
	                 const core::filearray_release_f& invalid_values_rf = []() {},
	                 const read_dict* invalid_values_dict = nullptr) const = 0;

  protected:
	/**
	 * Gets the db::type_t corresponding to this factory instance
	 *
	 * @return the factory type
	 */
	type_t type() const { return _type; }

  protected:
	type_t _type;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_ARRAY_FACTORY_INTERFACE_H__
