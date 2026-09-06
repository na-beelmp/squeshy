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
#include <pvcop/db/types.h>

#include <pvcop/db/array_factory.h>
#include <pvcop/db/types_manager.h>

#include <common/squey_assert.h>

static constexpr size_t SIZE = 200; //!< Number of data elements

/**
 * Check interface types can be use to create a db::array
 *
 */
int main()
{
	// Check memarray creation for each type
	for (pvcop::db::__impl::map_t const& t : pvcop::db::__impl::types_map) {
		pvcop::db::array data(pvcop::db::types_manager::array_factory(t.id).create_memarray(SIZE));
		PV_ASSERT_VALID(data.data() != nullptr);
	}
}
