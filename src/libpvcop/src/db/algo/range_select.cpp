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

#include <pvcop/db/algo.h>
#include <pvcop/db/array.h>
#include <pvcop/core/impl/map_reduce.h>

#include <pvlogger.h>

namespace pvcop::db::algo
{

void range_select(const db::array& array,
                  const std::string& min,
                  const std::string& max,
                  const db::selection& in_sel,
                  db::selection& out_sel)
{
	pvlogger::trace() << "db::algo::range_select(...)" << std::endl;

	assert(in_sel.size() == out_sel.size() || not in_sel);

	core::algo::__impl::map(in_sel, array.size(), [&](size_t pos, size_t len, size_t) {
		pvcop::db::selection in_sel_slice = in_sel.slice(pos, len);
		pvcop::db::selection out_sel_slice = out_sel.slice(pos, len);

		array.slice(pos, len).range_select(min, max, in_sel_slice, out_sel_slice);
	});
}
}
