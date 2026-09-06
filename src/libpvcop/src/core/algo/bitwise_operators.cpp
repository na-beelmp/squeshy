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

#include <pvcop/core/algo/bitwise_operators.h>
#include <pvcop/core/impl/bit.h>

static size_t chunk_count(const pvcop::core::selection& sel)
{
	return pvcop::core::__impl::bit_manip::chunk_count(sel.size());
}

pvcop::core::memarray<bool> pvcop::core::operator~(const pvcop::core::selection& lhs)
{
	pvcop::core::memarray<bool> result = lhs;
	for (size_t i = 0; i < chunk_count(result); i++) {
		result.data()[i] = ~result.data()[i];
	}

	return result;
}

pvcop::core::memarray<bool> pvcop::core::operator&(const core::selection& lhs,
                                                   const core::selection& rhs)
{
	pvcop::core::memarray<bool> result = lhs;
	result &= rhs;

	return result;
}

pvcop::core::memarray<bool> pvcop::core::operator|(const core::selection& lhs,
                                                   const core::selection& rhs)
{
	pvcop::core::memarray<bool> result = lhs;
	result |= rhs;

	return result;
}

pvcop::core::memarray<bool> pvcop::core::operator^(const core::selection& lhs,
                                                   const core::selection& rhs)
{
	pvcop::core::memarray<bool> result = lhs;
	result ^= rhs;

	return result;
}

pvcop::core::memarray<bool> pvcop::core::operator-(const core::selection& lhs,
                                                   const core::selection& rhs)
{
	pvcop::core::memarray<bool> result = lhs;
	result -= rhs;

	return result;
}

pvcop::core::selection& pvcop::core::operator&=(core::selection& lhs, const core::selection& rhs)
{
	assert(chunk_count(lhs) == chunk_count(rhs));

	for (size_t i = 0; i < chunk_count(lhs); i++) {
		lhs.data()[i] &= rhs.data()[i];
	}

	return lhs;
}

pvcop::core::selection& pvcop::core::operator|=(core::selection& lhs, const core::selection& rhs)
{
	assert(chunk_count(lhs) == chunk_count(rhs));

	for (size_t i = 0; i < chunk_count(lhs); i++) {
		lhs.data()[i] |= rhs.data()[i];
	}

	return lhs;
}

pvcop::core::selection& pvcop::core::operator^=(core::selection& lhs, const core::selection& rhs)
{
	assert(chunk_count(lhs) == chunk_count(rhs));

	for (size_t i = 0; i < chunk_count(lhs); i++) {
		lhs.data()[i] ^= rhs.data()[i];
	}

	return lhs;
}

pvcop::core::selection& pvcop::core::operator-=(core::selection& lhs, const core::selection& rhs)
{
	assert(chunk_count(lhs) == chunk_count(rhs));

	for (size_t i = 0; i < chunk_count(lhs); i++) {
		lhs.data()[i] &= ~rhs.data()[i];
	}

	return lhs;
}
