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

#include <bitset>

#include <pvlogger.h>

using namespace pvcop;

static constexpr size_t count = 2000000;

int main()
{
	core::memarray<bool> sel(count);

	size_t index = 0;
	size_t ratio = 1;

	for (size_t i = 0; i < sel.size(); i++) {
		sel[i] = static_cast<bool>(1);
	}

	pvlogger::info() << "ratio=" << ratio << ":" << std::endl;

	do {

		if (ratio > 1) {
			pvlogger::info() << "ratio=" << ratio << ":" << std::endl;

			for (size_t i = 0; i < sel.size(); i++) {
				sel[i] = static_cast<bool>(i % ratio == true);
			}
		}

		index = 0;
		for (size_t i = 0; i < sel.size();) {
			const size_t chunk_index = core::__impl::bit_manip::chunk_index(i);
			auto chunk = sel.data()[chunk_index];

			if (chunk == 0) {
				i += core::__impl::bit_manip::chunk_bit_size;
				continue;
			} else if (sel[i]) {
				i++;
			}
			i++;
		}

		index = 0;
		while (index < count) {
			size_t old_index = index;
			index = core::algo::find_first_set_bit(sel, index, count - 1);

			if (index == core::selection::INVALID_INDEX) {
				index = old_index;
			}

			index++;
		}

		ratio *= 2;
	} while (ratio <= 512);

	pvlogger::info() << "void compiler optimization (" << index << ")" << std::endl;
}
