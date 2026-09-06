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
#include <pvcop/core/selected_array.h>
#include <pvcop/db/algo.h>
#include <common/squey_assert.h>

#include <algorithm>
#include <chrono>
#include <regex>
#include <numeric>

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 200000000; //!< Number of data elements
#else
static constexpr size_t SIZE = 200000; //!< Number of data elements
#endif

static constexpr size_t MIN = 100;
static constexpr size_t MAX = (SIZE / 200) + MIN - 1;

using namespace pvcop;

int main()
{
	using type_t = uint32_t;
	std::string type = "number_uint32";

	db::array array(type, SIZE);
	core::array<type_t>& native_array = array.to_core_array<type_t>();
	std::iota(native_array.begin(), native_array.end(), 0);

	std::vector<std::string> values;
	values.resize(MAX - MIN + 1);
	for (size_t i = MIN; i <= MAX; i++) {
		values[i - MIN] = std::to_string(i);
	}

	pvcop::core::memarray<bool> out_sel(SIZE);
	pvcop::core::memarray<bool> out_sel2(SIZE);

	// db::algo::subselect
	auto start = std::chrono::system_clock::now();
	db::algo::subselect(array, db::algo::to_array(array, values), {}, out_sel);
	auto end = std::chrono::system_clock::now();

	std::transform(native_array.begin(), native_array.end(), native_array.begin(),
	               [](type_t val) { return val % MIN; });

	// db::array::subselect
	auto start2 = std::chrono::system_clock::now();
	array.subselect(array.to_array(std::vector<std::string>{std::to_string(0)}), {}, out_sel2);
	auto end2 = std::chrono::system_clock::now();

	std::chrono::duration<double> diff1 = end - start;
	std::chrono::duration<double> diff2 = end2 - start2;
#ifdef PVCOP_BENCH
	std::cout << diff1.count() + diff2.count();
#else
	std::cout << "Time to subselect " << SIZE << " elements with a search list of "
	          << (MAX - MIN + 1) << " elements : " << diff1.count() << " sec" << std::endl;
	std::cout << "Time to subselect " << SIZE
	          << " elements with a search list of one element : " << diff2.count() << " sec"
	          << std::endl;

	// db::algo::subselect
	const size_t bit_count = core::algo::bit_count(out_sel);
	PV_ASSERT_VALID(bit_count == (MAX - MIN + 1));
	bool valid_content = true;
	for (size_t i = 0; i < SIZE; i++) {
		valid_content &= (out_sel[i] == (i >= MIN && i <= MAX));
	}
	PV_ASSERT_VALID(valid_content);

	// db::array::subselect
	PV_ASSERT_VALID(pvcop::core::algo::bit_count(out_sel2) == (SIZE / MIN));
	valid_content = true;
	size_t i = 0;
	const auto& sel_array = core::make_selected_array(native_array, out_sel2);
	for (auto it = sel_array.begin(); it < sel_array.end(); ++it) {
		valid_content &= it.index() == (i++ * MIN);
	}
	PV_ASSERT_VALID(valid_content);

	// check multi-match subselect
	std::iota(native_array.begin(), native_array.end(), 0);
	std::vector<std::string> reg_exp = {".*0000$"};
	db::algo::subselect_if(array, reg_exp,
	                       [](const std::string& array_value, const std::string& exp_value) {
		                       return std::regex_match(array_value, std::regex(exp_value));
		                   },
	                       {}, out_sel);

	PV_VALID(core::algo::bit_count(out_sel), (SIZE / 10000) - 1);

	// check empty matching
	reg_exp = {};
	db::algo::subselect_if(array, reg_exp,
	                       [](const std::string& array_value, const std::string& exp_value) {
		                       return std::regex_match(array_value, std::regex(exp_value));
		                   },
	                       {}, out_sel);

	PV_VALID(core::algo::bit_count(out_sel), (size_t)0);

#endif // PVCOP_BENCH
}
