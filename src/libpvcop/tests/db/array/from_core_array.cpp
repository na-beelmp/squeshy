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

#include "db/array/array_impl.h"

#include <common/squey_assert.h>

#include <algorithm>

static constexpr size_t SIZE = 200; //!< Number of data elements

/**
 * Check we can convert memarray to array_impl with a default formatter
 */
int main()
{

	// Generate initial data
	pvcop::core::memarray<pvcop::db::index_t> data_array(SIZE, true);

	std::fill(data_array.begin(), data_array.end(), 12);

	pvcop::db::array_impl<pvcop::core::memarray, pvcop::db::index_t> res(
	    std::move(data_array),
	    pvcop::types::formatter_interface::sp(new pvcop::types::formatter_number<uint32_t>("")));

	char b[5];
	res.pvcop::db::array_impl_interface::at(0, b, 10);
}
