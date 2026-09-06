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
#include <pvcop/core/selection.h>
#include <pvcop/core/algo/selection.h>

#include <pvlogger.h>

static inline size_t bit_count(const uint64_t chunk)
{
	return __builtin_popcountll(chunk);
}
static inline size_t bit_count(const uint32_t chunk)
{
	return __builtin_popcount(chunk);
}

size_t pvcop::core::algo::__impl::find_nth_last_set_bit(
    const core::__impl::bit_manip::value_type* selection,
    int64_t n,
    const size_t start,
    const size_t end)
{
	assert(end >= start);

	using value_type = pvcop::core::__impl::bit_manip::value_type;

	const size_t start_index_chunk = core::__impl::bit_manip::chunk_index(start);
	const size_t end_index_chunk = core::__impl::bit_manip::chunk_index(end);
	const size_t end_index_bit = core::__impl::bit_manip::bit_index(end);

	size_t chunk_index = end_index_chunk;
	value_type chunk = selection[chunk_index];

	// clear previous bits
	if (end_index_bit != core::__impl::bit_manip::chunk_bit_size - 1) {
		chunk &= ((((value_type)1 << (end_index_bit + 1)) - 1));
	}
	n -= ::bit_count(chunk);

	while (n > 0 && chunk_index > start_index_chunk) {
		chunk = selection[--chunk_index];
		n -= ::bit_count(chunk);
	}

	if (n > 0) {
		return core::selection::INVALID_INDEX;
	}

	if (chunk_index == start_index_chunk) {
		const size_t start_index_bit = core::__impl::bit_manip::bit_index(start);
		n += ::bit_count(chunk & (((value_type)1 << (start_index_bit)) - 1));
		chunk &= ~(((value_type)1 << (start_index_bit)) - 1);
	}

	if (n > 0) {
		return core::selection::INVALID_INDEX;
	}

	size_t start_index = chunk_index * core::__impl::bit_manip::chunk_bit_size;

	int pos = __builtin_ctzll(chunk);

	while (n) {
		if (chunk & ((value_type)1 << (pos + 1))) {
			n++;
		}
		++pos;
	}
	return start_index + pos;
}

size_t
pvcop::core::algo::__impl::find_nth_set_bit(const core::__impl::bit_manip::value_type* selection,
                                            int64_t n,
                                            const size_t start,
                                            const size_t end)
{
	assert(end >= start);

	using value_type = pvcop::core::__impl::bit_manip::value_type;

	const size_t start_index_chunk = core::__impl::bit_manip::chunk_index(start);
	const size_t start_index_bit = core::__impl::bit_manip::bit_index(start);
	const size_t end_index_chunk = core::__impl::bit_manip::chunk_index(end);

	size_t chunk_index = start_index_chunk;
	core::__impl::bit_manip::value_type chunk = selection[chunk_index];

	// clear previous bits
	chunk &= ~(((value_type)1 << (start_index_bit)) - 1);
	n -= ::bit_count(chunk);

	while (n > 0 && chunk_index < end_index_chunk) {
		chunk = selection[++chunk_index];
		n -= ::bit_count(chunk);
	}

	if (n > 0) {
		return core::selection::INVALID_INDEX;
	}

	if (chunk_index == end_index_chunk) {
		const size_t end_index_bit = core::__impl::bit_manip::bit_index(end);
		if (end_index_bit != core::__impl::bit_manip::chunk_bit_size - 1) {
			n += ::bit_count(chunk & ~(((value_type)1 << (end_index_bit + 1)) - 1));
			chunk &= ((((value_type)1 << (end_index_bit + 1)) - 1));
		}
	}

	if (n > 0) {
		return core::selection::INVALID_INDEX;
	}

	size_t end_index = (chunk_index)*core::__impl::bit_manip::chunk_bit_size;

	int pos = 63 - __builtin_clzll(chunk);

	while (n) {
		if (chunk & ((value_type)1 << (pos - 1))) {
			n++;
		}
		--pos;
	}
	return end_index + pos;
}

size_t pvcop::core::algo::__impl::find_first_set_bit(
    const core::__impl::bit_manip::value_type* selection, const size_t start, const size_t end)
{
	using value_type = pvcop::core::__impl::bit_manip::value_type;

	const size_t start_index_chunk = core::__impl::bit_manip::chunk_index(start);
	const size_t start_index_bit = core::__impl::bit_manip::bit_index(start);
	const size_t end_index_chunk = core::__impl::bit_manip::chunk_index(end);

	size_t chunk_index = start_index_chunk;
	value_type chunk = selection[chunk_index];

	// clear previous bits
	chunk &= ~(((value_type)1 << (start_index_bit)) - 1);

	while (chunk == 0 && chunk_index < end_index_chunk) {
		chunk = selection[++chunk_index];
	}

	if (chunk_index == end_index_chunk && chunk != 0) {
		const size_t end_index_bit = core::__impl::bit_manip::bit_index(end);
		if (end_index_bit != core::__impl::bit_manip::chunk_bit_size - 1) {
			chunk &= ((((value_type)1 << (end_index_bit + 1)) - 1));
		}
	}

	if (chunk == 0) {
		return core::selection::INVALID_INDEX;
	}

	return chunk_index * core::__impl::bit_manip::chunk_bit_size + __builtin_ctzll(chunk);
}

size_t pvcop::core::algo::find_first_set_bit(const core::selection& sel,
                                             const size_t start,
                                             const size_t end)
{
	assert(end >= start);
	return __impl::find_first_set_bit(sel.data(), start, std::min(sel.size() - 1, end));
}

size_t pvcop::core::algo::find_nth_set_bit(const core::selection& sel,
                                           int64_t n,
                                           const size_t start,
                                           const size_t end)
{
	assert(end >= start);
	return __impl::find_nth_set_bit(sel.data(), n, start, std::min(sel.size() - 1, end));
}

size_t pvcop::core::algo::find_nth_last_set_bit(const core::selection& sel,
                                                int64_t n,
                                                const size_t start,
                                                const size_t end)
{
	return __impl::find_nth_last_set_bit(sel.data(), n, start, std::min(sel.size() - 1, end));
}

size_t pvcop::core::algo::bit_count(const core::__impl::bit_manip::value_type* s,
                                    size_t start /* = 0*/,
                                    size_t end /* = 0*/)
{
	using value_type = core::__impl::bit_manip::value_type;

	if (s == nullptr) {
		pvlogger::error() << "invalid selection" << std::endl;
		return 0;
	}

	assert(end >= start);

	const value_type* selection = s;
	const size_t start_chunk = core::__impl::bit_manip::chunk_index(start);
	const size_t end_chunk = core::__impl::bit_manip::chunk_index(end);
	constexpr static size_t mod_mask = core::__impl::bit_manip::chunk_bit_size - 1;

	if (start_chunk == end_chunk) {
		const value_type v0 = selection[start_chunk];
		const size_t shift0 = (start & mod_mask);
		const size_t shift1 = ((~end) & mod_mask);
		const value_type va = (v0 >> shift0) << shift0;
		const value_type vb = (v0 << shift1) >> shift1;

		return ::bit_count(va & vb);
	}

	size_t count = 0;
	const value_type* sel = selection + start_chunk + 1;
	const size_t n = end_chunk - start_chunk - 1;

	for (size_t i = 0; i < n; i++) {
		count += ::bit_count(sel[i]);
	}

	const value_type v0 = selection[start_chunk] >> (start & mod_mask);
	const value_type v1 = selection[end_chunk] << ((~end) & mod_mask);
	count += ::bit_count(v0);
	count += ::bit_count(v1);

	return count;
}

size_t
pvcop::core::algo::bit_count(const core::selection& s, size_t start /* = 0*/, size_t end /* = 0*/)
{

	start = start ? start : 0;
	end = end ? end : s.size() - 1;

	return pvcop::core::algo::bit_count(s.data(), start, end);
}
