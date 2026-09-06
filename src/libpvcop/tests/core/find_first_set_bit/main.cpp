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
#include <pvcop/core/algo/selection.h>

#include <common/squey_assert.h>

#include <pvlogger.h>

using namespace pvcop;

static constexpr size_t count = 128;
static constexpr size_t limit = 100;

int main()
{
	core::memarray<bool> sel(count);
	std::fill(sel.begin(), sel.end(), false);
	sel[limit] = true;

	PV_ASSERT_VALID(core::algo::find_first_set_bit(sel, 0, limit) == limit);

	sel[limit] = false;

	PV_ASSERT_VALID(core::algo::find_first_set_bit(sel, 0, limit) ==
	                core::selection::INVALID_INDEX);

	std::fill(sel.begin(), sel.end(), true);

	PV_ASSERT_VALID(core::algo::find_first_set_bit(sel, limit, limit) == limit);

	PV_ASSERT_VALID(core::algo::find_first_set_bit(sel, count - 1, count - 1) == count - 1);
}
