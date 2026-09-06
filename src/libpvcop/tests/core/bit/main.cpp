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

#include <pvcop/core/impl/bit.h>

#include <common/squey_assert.h>

#include <memory>

#define BIG_NUMBER 100000

using namespace pvcop::core::__impl;

int main()
{
	bit_manip::value_type bit_chunk = 0;
	std::unique_ptr<bit_manip::value_type[]> bit_chunks_storage(
	    new bit_manip::value_type[BIG_NUMBER]);
	bit_manip::value_type* bit_chunks = bit_chunks_storage.get();

	size_t bit_per_chunk = bit_manip::chunk_bit_size;
	size_t byte_per_chunk = bit_manip::chunk_byte_size;

	/* checking bit constexpr
	 */
	PV_VALID(byte_per_chunk, sizeof(bit_manip::value_type));
	PV_VALID(bit_per_chunk, sizeof(bit_manip::value_type) * 8);

	/* checking bit::chunk_index(size_t)
	 */
	for (size_t i = 0; i < BIG_NUMBER; ++i) {
		size_t expected = i / bit_per_chunk;
		PV_VALID(bit_manip::chunk_index(i), expected, "i", i);
	}

	/* checking bit_manip::bit_index(size_t)
	 */
	for (size_t i = 0; i < BIG_NUMBER; ++i) {
		size_t expected = i % bit_per_chunk;
		PV_VALID(bit_manip::bit_index(i), expected, "i", i);
	}

	/* checking bit_manip::chunk_count(size_t)
	 */
	PV_VALID(bit_manip::chunk_count(0), (size_t)0, "i", 0);
	for (size_t i = 1; i < BIG_NUMBER; ++i) {
		size_t expected = ((i - 1) / bit_per_chunk) + 1;
		PV_VALID(bit_manip::chunk_count(i), expected, "i", i);
	}

	/* checking bit_manip::to_mem_size(size_t)
	 */
	PV_VALID(bit_manip::to_mem_size(0), (size_t)0, "i", 0);
	for (size_t i = 1; i < BIG_NUMBER; ++i) {
		size_t expected = (((i - 1) / bit_per_chunk) + 1) * sizeof(bit_manip::value_type);
		PV_VALID(bit_manip::to_mem_size(i), expected, "i", i);
	}

	// no need to check ::index() and ::chunk() which are obvious

	/* checking bit constructor
	 */
	for (size_t i = 1; i < BIG_NUMBER; ++i) {
		const bit_manip::value_type* expected_chunk = bit_chunks + bit_manip::chunk_index(i);
		size_t expected_index = bit_manip::bit_index(i);
		const bit b(bit_chunks, i);
		PV_VALID(b.chunk(), expected_chunk, "i", i);
		PV_VALID(b.index(), expected_index, "i", i);
	}

	/* checking bit::mask()
	 */
	for (size_t i = 0; i < BIG_NUMBER; ++i) {
		size_t expected = (size_t)1 << (i % bit_per_chunk);
		bit b(&bit_chunk, i);
		PV_VALID(b.mask(), expected, "i", i);
	}

	/* checking bit::operator=(bool)
	 */
	for (size_t i = 0; i < BIG_NUMBER; ++i) {
		bit_manip::value_type& chunk = bit_chunks[bit_manip::chunk_index(i)];

		chunk = 0;
		uint64_t expected = (uint64_t)1 << bit_manip::bit_index(i);

		bit b(bit_chunks, i);
		b = true;

		PV_VALID(chunk, expected, "i", i);
	}

	/* checking bit::operator=(bit)
	 */
	for (size_t i = 0; i < bit_per_chunk; ++i) {
		bit_chunk = 0;
		bit bit_ref(&bit_chunk, i);
		bit_ref = true;

		for (size_t j = 0; j < BIG_NUMBER; ++j) {
			bit_manip::value_type& chunk = bit_chunks[bit_manip::chunk_index(j)];
			chunk = 0;
			uint64_t expected = (uint64_t)1 << bit_manip::bit_index(j);

			bit b(bit_chunks, j);
			b = bit_ref;

			PV_VALID(chunk, expected, "j", j);
		}
	}

	/* checking bit::operator==(bit)
	 */
	{
		bit_chunk = 0;
		bit bit_ref(&bit_chunk, 3);
		bit_ref = true;

		for (size_t i = 0; i < bit_per_chunk; ++i) {
			bit b(&bit_chunk, i);

			if (i == 3) {
				PV_VALID(b == bit_ref, true, "i", i);
			} else {
				PV_VALID(b == bit_ref, false, "i", i);
			}
		}
	}

	/* checking bit::operator!=(bit)
	 */
	{
		bit_chunk = 0;
		bit bit_ref(&bit_chunk, 3);
		bit_ref = true;

		for (size_t i = 0; i < bit_per_chunk; ++i) {
			bit b(&bit_chunk, i);

			if (i == 3) {
				PV_VALID(b != bit_ref, false, "i", i);
			} else {
				PV_VALID(b != bit_ref, true, "i", i);
			}
		}
	}

	/* checking bit::operator++()
	 */
	{
		bit_iterator b(bit_chunks, 0);

		for (size_t i = 0; i < BIG_NUMBER; ++i) {
			bit_iterator b_ref(bit_chunks, i);
			PV_ASSERT_VALID(b == b_ref, "i", i);
			++b;
		}
	}

	/* checking bit::operator+(size_t)
	 */
	{
		bit_iterator b_ref(bit_chunks, 0);
		bit_iterator b_base(bit_chunks, 0);

		for (size_t i = 0; i < BIG_NUMBER; ++i) {
			bit_iterator b_i = b_base + i;
			PV_ASSERT_VALID(b_i == b_ref, "i", i);
			++b_ref;
		}
	}

	/* checking bit::operator+=(size_t)
	 */
	{
		bit_iterator b_ref(bit_chunks, 0);
		bit_iterator b_base(bit_chunks, 0);

		for (size_t i = 0; i < BIG_NUMBER; ++i) {
			bit_iterator b_i = b_base;
			b_i += i;
			PV_ASSERT_VALID(b_i == b_ref, "i", i);
			++b_ref;
		}
	}

	/* checking bit::operator*()
	 */
	{
		bit_iterator b(bit_chunks, 0);
		std::unique_ptr<bit> bp(new bit(bit_chunks, 0));

		PV_VALID(*b != *bp, false);
	}

	/* checking bit::operator bool()
	 */
	{
		bit_chunk = 0;
		bit bit_ref(&bit_chunk, 3);
		bit_ref = true;

		for (size_t i = 0; i < bit_per_chunk; ++i) {
			bit b(&bit_chunk, i);

			if (i == 3) {
				PV_VALID(b.operator bool(), true, "i", i);
			} else {
				PV_VALID(b.operator bool(), false, "i", i);
			}
		}
	}

	/* checking... (sigh) it's finished \o/
	 */

	return 0;
}
