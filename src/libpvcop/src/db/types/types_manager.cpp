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

#include <pvcop/db/types_manager.h>

#include <pvcop/db/array_factory.h>
#include <pvcop/db/array_impl_interface.h>
#include <db/array/array_impl.h>

#include <pvlogger.h>

pvcop::db::types_manager::types_manager()
= default;

pvcop::db::types_manager::~types_manager()
{
	for (auto it : get()._type_interfaces) {
		delete it.second.traits;
		delete it.second.array_factory;
	}
}

pvcop::db::types_manager& pvcop::db::types_manager::get()
{
	static types_manager instance;

	return instance;
}

pvcop::db::array_factory_interface& pvcop::db::types_manager::array_factory(pvcop::db::type_t type)
{
	return *get()._type_interfaces[type].array_factory;
}

pvcop::db::traits_interface& pvcop::db::types_manager::traits(pvcop::db::type_t type)
{
	return *get()._type_interfaces[type].traits;
}
