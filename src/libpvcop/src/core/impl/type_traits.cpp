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

#include <pvcop/core/impl/type_traits.h>

#include <cstring>

namespace std
{

using namespace pvcop::core::__impl;

void fill(bit_iterator begin, bit_iterator end, bool val)
{
	using value_type = bit_manip::value_type;
	const unsigned char mem_pattern = 0xFF >> ((not val) << 4);

	const size_t bytes_per_chunk = sizeof(value_type) / sizeof(mem_pattern);

	ptrdiff_t chunk_count = end.chunk() - begin.chunk() + (end.index() > 0);

	auto rev_begin_index = bit_manip::chunk_bit_size - begin.index();
	auto rev_end_index = bit_manip::chunk_bit_size - end.index();

	if (chunk_count == 1) {
		*begin.chunk() = val ? (*begin.chunk() |
		                        ((end.index() ? ((value_type)1 << (end.index() - begin.index())) : 0) - 1)
		                            << begin.index())
		                     : (*begin.chunk() &
		                        ~(((end.index() ? ((value_type)1 << (end.index() - begin.index())) : 0) - 1)
		                          << begin.index()));
		return;
	}

	if (begin.index() > 0) {
		*begin.chunk() = val ? (*begin.chunk() | (((value_type)1 << rev_begin_index) - 1)) << begin.index()
		                     : (*begin.chunk() & ((value_type)-1 >> rev_begin_index));
	}
	if (end.index() > 0) {
		*end.chunk() = val ? (*end.chunk() | ((value_type)-1 >> rev_end_index))
		                   : (*end.chunk() & ~(((value_type)1 << end.index()) - 1));
	}

	std::memset(begin.chunk() + (begin.index() > 0), mem_pattern,
	            ((chunk_count - ((end.index() > 0) + (begin.index() > 0)))) * bytes_per_chunk);
}

} // namespace std
