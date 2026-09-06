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

#include <pvcop/db/array.h>
#include <pvcop/core/memarray.h>
#include <pvcop/db/algo.h>
#include <common/squey_assert.h>

#include <algorithm>
#include <chrono>

static constexpr size_t SIZE = 200000000; //!< Number of data elements
static constexpr size_t VALUE = 666;

using namespace pvcop;

int main()
{
	using type = uint64_t;

	db::array array(pvcop::db::type_traits::type<type>::get_type_id(), SIZE);
	core::array<type>& native_array = array.to_core_array<type>();
	std::iota(native_array.begin(), native_array.end(), 0);

	std::vector<std::string> values = {std::to_string(VALUE)};

	core::memarray<bool> out_sel(SIZE);

	// db::algo::subselect
	auto start = std::chrono::system_clock::now();
	db::algo::subselect(array, values, db::selection(), out_sel);
	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> diff = end - start;
	std::cout << diff.count();
}
