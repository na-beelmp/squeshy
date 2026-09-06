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

#include <common/squey_assert.h>

#include <algorithm>
#include <numeric>

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 100000000;
#else
static constexpr size_t SIZE = 100000;
#endif

int main()
{
	pvcop::db::array array_in_1("number_uint64", SIZE);
	pvcop::db::array array_in_2("number_uint64", SIZE);

	auto& array_in_1_native = array_in_1.to_core_array<uint64_t>();
	auto& array_in_2_native = array_in_2.to_core_array<uint64_t>();

	std::iota(array_in_1_native.begin(), array_in_1_native.end(), 0);
	std::iota(array_in_2_native.begin(), array_in_2_native.end(), array_in_1_native.size());

	auto start = std::chrono::system_clock::now();

	pvcop::db::array array_out_1 = array_in_1.concat(array_in_2);

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;

#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "concat with " << SIZE << " elements took: " << diff.count() << " sec"
	          << std::endl;

	const auto& array_out_1_native = array_out_1.to_core_array<uint64_t>();

	PV_ASSERT_VALID(array_out_1.size() == (array_in_1.size() + array_in_2.size()));

	bool res = std::all_of(array_out_1_native.begin(), array_out_1_native.end(),
	                       [&](uint64_t i) { return i == array_out_1_native[i]; });
	PV_ASSERT_VALID(res);
#endif
}
