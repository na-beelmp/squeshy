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

#include <pvcop/core/memarray.h>
#include <pvcop/core/selection.h>

#include <common/squey_assert.h>

#include <pvlogger.h>

#include <algorithm>
#include <chrono>
#include <numeric>

using namespace pvcop;

static constexpr size_t SIZE = 1000;
static constexpr size_t SEL_RATIO = 20;
static constexpr size_t RANGE = SIZE / SEL_RATIO;

int main()
{
	using type = size_t;

	core::memarray<type> array(SIZE);
	std::iota(array.begin(), array.end(), 0);

	core::memarray<bool> sel(SIZE);
	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = (bool)not(i % RANGE);
	}

	/**
	 * Test selected_array auto-for loop
	 */
	size_t sum = 0;
	for (type v : core::make_selected_array(array, sel)) {
		sum += v;
	}
	PV_VALID(sum, ((((SEL_RATIO - 1)) * SIZE) / 2));

	/**
	 * Test selected_array iterator based loop
	 */
	bool res = true;
	size_t i = 0;
	sum = 0;
	const auto& sel_array = core::make_selected_array(array, sel);
	for (auto it = sel_array.begin(); it < sel_array.end(); ++it) {
		res &= it.index() == (i++ * RANGE);
		sum += *it;
	}
	PV_VALID(sum, ((((SEL_RATIO - 1)) * SIZE) / 2));
	PV_ASSERT_VALID(res);
}
