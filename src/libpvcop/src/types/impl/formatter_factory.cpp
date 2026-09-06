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

#include <pvcop/types/impl/formatter_factory.h>
#include <pvcop/types/exception.h>

#include <pvcop/types/datetime.h>
#include <pvcop/types/datetime_ms.h>
#include <pvcop/types/datetime_us.h>
#include <pvcop/types/duration.h>
#include <pvcop/types/ipv4.h>
#include <pvcop/types/ipv6.h>
#include <pvcop/types/mac_address.h>
#include <pvcop/types/number.h>
#include <pvcop/types/string.h>

static void formatter_factory_init() __attribute__((constructor));

void formatter_factory_init()
{
	pvcop::types::__impl::formatter_factory::formatter_factory::get();
}

pvcop::types::__impl::formatter_factory::formatter_factory()
{
	register_type<pvcop::types::formatter_datetime>();
	register_type<pvcop::types::formatter_datetime_ms>();
	register_type<pvcop::types::formatter_datetime_us>();
	register_type<pvcop::types::formatter_duration>();
	register_type<pvcop::types::formatter_ipv4>();
	register_type<pvcop::types::formatter_ipv6>();
	register_type<pvcop::types::formatter_mac_address>();
	register_type<pvcop::types::formatter_number>(pvcop::types::numbers_t());
	register_type<pvcop::types::formatter_string>();
}

pvcop::types::formatter_interface*
pvcop::types::__impl::formatter_factory::create(const std::string& formatter,
                                                const std::string& parameters /*= std::string()*/)
{
	const auto& iter = get()._map.find(formatter);

	if (iter != get()._map.end()) {
		return iter->second->create(parameters);
	}

	throw pvcop::types::exception::format_error(
	    "formatter_factory: trying to create formatter of unknown type '" + formatter + "'\n" +
	    "Did you forgot to register that type using pvcop::register_type?\n");
}

pvcop::db::type_t pvcop::types::__impl::formatter_factory::type(const std::string& name)
{
	const auto& iter = get()._map.find(name);

	if (iter != get()._map.end()) {
		return iter->second->type();
	} else {
		return "";
	}
}

pvcop::types::__impl::formatter_factory& pvcop::types::__impl::formatter_factory::get()
{
	static formatter_factory instance;

	return instance;
}
