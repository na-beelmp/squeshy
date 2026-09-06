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

static constexpr size_t SIZE = 1000;
static constexpr size_t SEL_RATIO = 10;
static constexpr size_t RANGE = SIZE / SEL_RATIO;

int main()
{
	pvcop::core::memarray<bool> sel(SIZE);

	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = (bool)not(i % RANGE);
	}

	size_t sum = 0;
	const auto& sel_array = pvcop::core::make_selected_array(sel, sel);
	for (auto it = sel_array.begin(); it < sel_array.end(); ++it) {
		sum += it.index();
	}
	PV_VALID(sum, ((((SEL_RATIO - 1)) * SIZE) / 2));

	/**
	 * Bitwise complement
	 */
	const pvcop::core::selection& complement_sel = ~sel;

	size_t inverted_sum = 0;
	const auto& inverted_sel_array =
	    pvcop::core::make_selected_array(complement_sel, complement_sel);
	for (auto it = inverted_sel_array.begin(); it < inverted_sel_array.end(); ++it) {
		inverted_sum += it.index();
	}
	PV_VALID(complement_sel.size(), SIZE);
	PV_VALID(inverted_sum, ((((SIZE - 1)) * SIZE) / 2) - sum);

	// PV_ASSERT_VALID(pvcop::core::make_selected_array(complement_invalid_sel,
	// complement_invalid_sel).size() == SIZE);
}
