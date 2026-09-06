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

#include <common/squey_assert.h>

#include <chrono>
#include <random>
#include <algorithm>

static constexpr size_t SIZE = 200000; //!< Number of data elements

/**
 * Check db::array.group function
 */
int main()
{

	// Generate initial data
	pvcop::db::array data("number_uint64", SIZE);
	auto& data_array = data.to_core_array<uint64_t>();

	std::random_device rd;
	std::mt19937 g(rd());
	std::uniform_int_distribution<> dis(0, 100000);
	std::generate(data_array.begin(), data_array.end(), [&dis, &g]() { return dis(g); });

	{
		// Declare outputs
		pvcop::db::groups group;
		pvcop::db::extents extents;

		auto start = std::chrono::system_clock::now();

		// Do computation
		data.group(group, extents);

		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = end - start;
		std::cout << "Time to group " << SIZE << "elements: " << diff.count() << " s" << std::endl;

		// Check output sizes
		PV_VALID(group.size(), SIZE);
		PV_ASSERT_VALID(extents.size() < SIZE);

		const auto& group_array = group.to_core_array();
		const auto& extents_array = extents.to_core_array();

		for (size_t i = 0; i < SIZE; i++) {
			// Check all groups are valid
			PV_ASSERT_VALID(group_array[i] < extents.size());
			// Check every elements match the "top" of the group
			PV_ASSERT_VALID(data_array[i] == data_array[extents_array[group_array[i]]]);
		}
	}

	{
		// Same with selection
		pvcop::core::memarray<bool> sel(SIZE);
		std::fill(std::fill_n(sel.begin(), SIZE / 2, true), sel.end(), false);

		// Declare outputs
		pvcop::db::groups group_sel;
		pvcop::db::extents extents_sel;

		auto start = std::chrono::system_clock::now();

		// Do computation
		data.group(group_sel, extents_sel, sel);

		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = end - start;
		std::cout << "Time to group " << SIZE << "elements: " << diff.count() << " s" << std::endl;

		const auto& group_array_sel = group_sel.to_core_array();
		const auto& extents_array_sel = extents_sel.to_core_array();

		// Check output sizes
		PV_ASSERT_VALID(group_sel.size() == SIZE / 2);
		PV_ASSERT_VALID(extents_sel.size() < SIZE / 2);

		for (size_t i = 0; i < SIZE / 2; i++) {
			// Check all groups are valid
			PV_ASSERT_VALID(group_array_sel[i] < extents_sel.size());
			// Check every elements match the "top" of the group
			PV_ASSERT_VALID(data_array[i] == data_array[extents_array_sel[group_array_sel[i]]]);
		}
	}
}
