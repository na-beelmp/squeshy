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

#ifndef __PVCOP_DB_TYPES_MANAGER_H__
#define __PVCOP_DB_TYPES_MANAGER_H__

#include <pvcop/db/array_factory_interface.h>
#include <pvcop/db/array_factory.h>
#include <db/types/traits_interface.h>

#include <unordered_map>

namespace pvcop
{

namespace db
{

/**
 * Singleton class intended to dynamically handle types at runtime.
 */
class types_manager
{
  private:
	struct type_interface {
		type_interface() : array_factory(nullptr), traits(nullptr) {}
		type_interface(array_factory_interface* f, traits_interface* t)
		    : array_factory(f), traits(t)
		{
		}

		array_factory_interface* array_factory;
		traits_interface* traits;
	};

  public:
	/**
	 * Get a reference to the factory of the provided type
	 *
	 * @param type type_id of the factory
	 */
	static array_factory_interface& array_factory(pvcop::db::type_t type);

	/**
	 * Get a reference to the traits_interface of the provided type
	 *
	 * @param type type_id of the traits_interface
	 */
	static traits_interface& traits(pvcop::db::type_t type);

  public:
	/**
	 * Register a new type
	 *
	 * @param Type array elements type
	 */
	template <typename Type>
	void register_type(const type_t& type)
	{
		_type_interfaces.emplace(
		    type, type_interface(new db::array_factory<Type>(type), new db::traits<Type>()));
	}

	/**
	 * Singleton access
	 *
	 * @return a reference to the types_manager static object
	 */
	static types_manager& get();

  private:
	/**
	 * Default constructor
	 *
	 * register all the supported types
	 */
	types_manager();

	/**
	 * Default destructor
	 *
	 * deallocate the resources acquired by register_type
	 */
	~types_manager();

	types_manager(const types_manager&) = delete;
	types_manager& operator=(const types_manager&) = delete;

	types_manager& operator=(const types_manager&&) = delete;
	types_manager(types_manager&&) = delete;

  private:
	std::unordered_map<std::string, type_interface> _type_interfaces;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_TYPES_MANAGER_H__
