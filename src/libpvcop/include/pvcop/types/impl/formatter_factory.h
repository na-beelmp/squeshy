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

#ifndef __PVCOP_TYPES_IMPL_FORMATTER_FACTORY_H__
#define __PVCOP_TYPES_IMPL_FORMATTER_FACTORY_H__

#include <pvcop/db/types.h>
#include <pvcop/db/types_manager.h>
#include <pvcop/types/formatter/formatter_interface.h>
#include <pvcop/types/formatter/formatter.h>

#include <pvlogger.h>

#include <unordered_map>
#include <memory>
#include <tuple>
#include <type_traits>

namespace pvcop
{

namespace types
{

namespace __impl
{

template <class Base>
class formatter_creator_interface
{
  public:
	virtual ~formatter_creator_interface() {}

  public:
	virtual Base* create(const std::string& parameters) = 0;

	virtual db::type_t type() = 0;
};

template <class Base, class Derived>
class formatter_creator : public formatter_creator_interface<Base>
{
  public:
	Base* create(const std::string& parameters) override { return new Derived(parameters.c_str()); }

	db::type_t type() override
	{
		Derived f("");
		return f.name();
	}
};

class formatter_factory
{
  public:
	template <typename Formatter>
	bool register_type()
	{
		static_assert(std::is_base_of<pvcop::types::formatter_interface, Formatter>::value,
		              "type is not derived from pvcop::types::formatter_interface.");

		Formatter formatter("");

		db::type_t type(formatter.name());

		map_t::const_iterator iter = _map.find(type);

		if (iter != _map.end()) {
			// Type already registred, we don't need to save it one more time.
			return false;
		}

		_map[type].reset(new formatter_creator<pvcop::types::formatter_interface, Formatter>());

		pvcop::db::types_manager::get().register_type<typename Formatter::type>(type);

		return true;
	}

	template <template <typename> class Formatter, typename Tuple>
	void register_type(const Tuple& types)
	{
		pvcop::core::__impl::visit_types(
		    [&](auto type) { this->register_type<Formatter<decltype(type)>>(); }, types);
	}

  public:
	static pvcop::types::formatter_interface* create(const std::string& formatter,
	                                                 const std::string& parameters = std::string());

	static pvcop::db::type_t type(const std::string& name);

  public:
	static formatter_factory& get();

  private:
	formatter_factory();

	formatter_factory(const formatter_factory&) = delete;
	formatter_factory& operator=(const formatter_factory&) = delete;

	formatter_factory& operator=(const formatter_factory&&) = delete;
	formatter_factory(formatter_factory&&) = delete;

  private:
	using key_t = std::string;
	using map_t = std::unordered_map<
	    key_t,
	    std::unique_ptr<formatter_creator_interface<pvcop::types::formatter_interface>>>;

	map_t _map;
};

} // namespace pvcop::types::__impl

} // namespace pvcop::types

} // namespace pvcop

#endif // __PVCOP_TYPES_IMPL_FORMATTER_FACTORY_H__
