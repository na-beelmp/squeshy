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

#ifndef __PVCOP_TYPES_FACTORY_H__
#define __PVCOP_TYPES_FACTORY_H__

#include <pvcop/db/types.h>

namespace pvcop
{

namespace types
{

class formatter_interface;

/**
 * This class provides static methods to register new formatter or retrieve informations
 * from their name.
 */
class factory
{
  public:
#if 0 // this need to distribute basically all pvcop source code...
	/**
	 * Register a new formatter class
	 *
	 * @tparam Formatter a class derived from pvcop::types::formatter_interface
	 *
	 * @return @c true if the formatter has been registered; @c false otherwise.
	 */
	template <template <typename> class Formatter>
	static bool register_type()
	{
		return __impl::formatter_factory::get().register_type<Formatter>();
	}
#endif

	/**
	 * Checks if a formatter is registered using its name
	 *
	 * @param name the formatter name
	 *
	 * @return true if the formatter is registered; false otherwise
	 */
	static bool is_registered(const std::string& name);

	/**
	 * Creates a new formatter instance using its name and its parameters
	 *
	 * @param name the formatter name
	 * @param parameters the formatter parameters string
	 *
	 * @return a pointer on the new parameter
	 */
	static formatter_interface* create(const std::string& name, const std::string& parameters);
};

} // namespace pvcop::types

} // namespace pvcop

#endif // __PVCOP_TYPES_FACTORY_H__
