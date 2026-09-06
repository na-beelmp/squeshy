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
#include <pvcop/types/factory.h>

#include "db/array/array_impl.h"

#include <common/squey_assert.h>

#include <chrono>

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 10000000; //!< Number of data elements
#else
static constexpr size_t SIZE = 200; //!< Number of data elements
#endif

/**
 * Check we can convert memarray to array_impl with a default formatter
 */
int main()
{
	using type_t = uint32_t;
	std::string type = "number_uint32";

	pvcop::db::array ref(type, SIZE);
	pvcop::core::array<type_t>& ref_array = ref.to_core_array<type_t>();
	std::iota(ref_array.begin(), ref_array.end(), 0);

	auto formatter_ipv4 = std::shared_ptr<pvcop::types::formatter_interface>(
	    pvcop::types::factory::create("ipv4", ""));
	ref.set_formatter(formatter_ipv4);

	auto start = std::chrono::system_clock::now();

	pvcop::db::array res = ref.copy();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
	std::cout << diff.count();

	pvcop::core::array<type_t>& res_array = res.to_core_array<type_t>();

	PV_VALID(res_array.size(), ref_array.size());

#ifndef PVCOP_BENCH
	for (size_t i = 0; i < res_array.size(); i++) {
		PV_VALID(res_array[i], ref_array[i]);
		PV_VALID(res_array.at(i), ref_array.at(i));
	}

	res_array[0] = 12;
	PV_VALID(ref_array[0], 0U);
#endif
}
