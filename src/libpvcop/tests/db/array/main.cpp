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

#include <pvcop/pvcop.h>

#include <common/squey_assert.h>
#include <pvlogger.h>

using namespace pvcop;

constexpr const size_t count = 42;

void check_default_ctor(db::array& array)
{
	PV_VALID((bool)array, false);
	PV_VALID(array.size(), 0UL);
	PV_VALID((bool)array.data(), false);
	PV_VALID(array.type(), "");
}

void check_default_ctor()
{
	db::array array;

	check_default_ctor(array);
}

void check_size_ctor(db::type_t type_id, size_t size, db::array& array)
{
	PV_ASSERT_VALID((bool)array);
	PV_VALID(array.size(), size);
	PV_VALID(array.type(), type_id);
	PV_ASSERT_VALID((bool)array.data());
}

void check_size_ctor(db::type_t type_id, size_t size)
{
	db::array array(type_id, size);

	check_size_ctor(type_id, size, array);
}

void check_move_ctor(db::type_t type_id, size_t size)
{
	db::array array1(type_id, size);
	db::array array2;

	array2 = std::move(array1);

	check_default_ctor(array1);
	check_size_ctor(type_id, size, array2);
}

struct testsuite {
	template <typename T>
	void operator()(T) const
	{
		db::type_t type_id = pvcop::db::type_traits::type<T>::get_type_id();

		check_size_ctor(type_id, count);
		check_move_ctor(type_id, count);
	}
};

int main()
{
	check_default_ctor();

	db::visit_supported_types(testsuite());

	return 0;
}
