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

#ifdef PVCOP_BENCH
static constexpr size_t SIZE = 200000000; //!< Number of data elements
#else
static constexpr size_t SIZE = 200000; //!< Number of data elements
#endif

/**
 * Check db::array.join function
 *
 * This test join on extents from a group.
 */
int main()
{
	// Initialise random
	std::random_device rd;
	std::mt19937 g(rd());

	// Generate data to group
	std::uniform_int_distribution<uint64_t> dis_sparse(0, SIZE / 10e4);
	pvcop::db::array gdata("number_uint64", SIZE);
	auto& gdata_array = gdata.to_core_array<uint64_t>();
	std::generate(gdata_array.begin(), gdata_array.end(),
	              [&dis_sparse, &g]() { return dis_sparse(g); });

	pvcop::core::memarray<bool> sel(gdata.size(), true);

	// Compute grouping
	pvcop::db::groups group;
	pvcop::db::extents extents;

	gdata.group(group, extents);

	for (uint64_t v : extents.to_core_array()) {
		sel[v] = true;
	}

	auto start = std::chrono::system_clock::now();

	// Do computation
	pvcop::db::array join1 = gdata.join(extents);
	pvcop::db::array join2 = gdata.join(sel);

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
	std::cout << "Time to join " << SIZE << " elements: " << diff.count() << " s" << std::endl;

	auto& join_array = join1.to_core_array<uint64_t>();

	// Check output sizes
	PV_ASSERT_VALID(extents.size() == join1.size());

	// Extents contains all groups so join on extents give only unique elements.
	PV_ASSERT_VALID(std::unique(join_array.begin(), join_array.end()) == join_array.end());

	PV_ASSERT_VALID(join1 == join2);
#endif
}
