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
#include <pvcop/db/array.h>
#include <pvcop/db/algo.h>

#include <common/squey_assert.h>

using namespace pvcop;

static constexpr size_t count = 1000;

/**
 * Check that std::fill specialization for pvcop::core::array<bool> works correctly
 */
int main()
{
	/*
	 * Check that no bits are set on the whole array
	 */
	core::memarray<bool> sel(count, true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);

	/*
	 * Check that all bits are set on the whole array
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);

	/*
	 * Check clearing bits everywhere except in the begining and the end
	 */
	std::fill(std::next(sel.begin(), 10), std::next(sel.begin(), count - 7), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)17);
	for (size_t i = 10; i < count - 7; i++) {
		PV_ASSERT_VALID(sel[i] == false);
	}

	/*
	 * Check setting bits everywhere except in the begining and the end
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), 10), std::next(sel.begin(), count - 7), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count - 17UL);
	for (size_t i = 10; i < count - 7; i++) {
		PV_ASSERT_VALID(sel[i] == true);
	}

	/*
	 * Check that all bits are set on exactly two chunks
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), 64), std::next(sel.begin(), 192), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)128);

	/*
	 * Check that all bits are cleared on exactly two chunks
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(std::next(sel.begin(), 64), std::next(sel.begin(), 192), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count - 128UL);

	/*
	 * Check that all bits are set on exactly a chunk
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), 64), std::next(sel.begin(), 128), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)64);

	/*
	 * Check that all bits are set on two chunks
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), 200), std::next(sel.begin(), 300), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)100);
	for (size_t i = 200; i < 300; i++) {
		PV_ASSERT_VALID((bool)sel[i]);
	}

	/*
	 * Check that all bits are cleared on two chunks
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(std::next(sel.begin(), 200), std::next(sel.begin(), 300), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count - 100UL);
	for (size_t i = 200; i < 300; i++) {
		PV_ASSERT_VALID(sel[i] == false);
	}

	/*
	 * Check that all bits are set on exactly a chunk
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), 64), std::next(sel.begin(), 128), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)64);

	/*
	 * Check that all bits are cleared on exactly a chunk
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(std::next(sel.begin(), 64), std::next(sel.begin(), 128), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count - 64UL);

	/*
	 * Check that all bits are set on less than a chunk
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), 200), std::next(sel.begin(), 224), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)24);
	for (size_t i = 200; i < 224; i++) {
		PV_ASSERT_VALID(sel[i] == true);
	}

	/*
	 * Check that all bits are cleared on less than a chunk
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(std::next(sel.begin(), 200), std::next(sel.begin(), 224), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count - 24UL);
	for (size_t i = 200; i < 224; i++) {
		PV_ASSERT_VALID(sel[i] == false);
	}

	/*
	 * Check that a single bit is set
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), 100), std::next(sel.begin(), 101), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)1);
	PV_ASSERT_VALID((bool)sel[100]);

	/*
	 * Check that a single bit is cleared
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(std::next(sel.begin(), 100), std::next(sel.begin(), 101), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count - 1);
	PV_ASSERT_VALID(not sel[100]);

	/*
	 * Check that a single bit is set at the begining of the array
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(sel.begin(), std::next(sel.begin(), 1), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)1);
	PV_ASSERT_VALID((bool)sel[0]);

	/*
	 * Check that a single bit is cleared at the begining of the array
	 */
	std::fill(sel.begin(), std::next(sel.begin(), 1), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);

	/*
	 * Check that a single bit is set at the end of the array
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(std::next(sel.begin(), count - 1), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)1);
	PV_VALID((bool)sel[count - 1], true);

	/*
	 * Check that a single bit is cleared at the end of the array
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(std::next(sel.begin(), count - 1), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count - 1);
	PV_ASSERT_VALID(not sel[count - 1]);

	/*
	 * Check that no bits changed when clearing bits
	 */
	std::fill(sel.begin(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(sel.begin(), sel.begin(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);
	std::fill(sel.end(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), count);

	/*
	 * Check that no bits changed when setting bits
	 */
	std::fill(sel.begin(), sel.end(), false);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(sel.begin(), sel.begin(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
	std::fill(sel.end(), sel.end(), true);
	PV_VALID(pvcop::core::algo::bit_count(sel), (size_t)0);
}
