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

#include <algorithm>
#include <type_traits>

namespace pvcop::db::algo
{

namespace __impl
{

template <typename T>
using reduce_operation_t = T (db::array_base::*)(const db::selection&) const;

/**
 * Helper function to merge the results of parallel reductions, in order to apply a final reduction
 */
template <typename T>
static db::array merge_op(std::vector<T>& partial_results, type_t type)
{
	db::array final_array(type, partial_results.size());
	core::array<T>& native_final_array = final_array.to_core_array<T>();

	std::copy_n(partial_results.begin(), partial_results.size(), native_final_array.begin());

	return final_array;
}

/**
 * Specialization for reductions returning a db::array
 */
template <>
db::array merge_op(std::vector<db::array>& partial_results, type_t /*type*/)
{
	return db::array::concat(partial_results);
}

/**
 * Execute a reduce operation in parallel
 */
template <typename T>
T reduce_operation(const db::array& in_array,
                   reduce_operation_t<T> reduce_op,
                   const db::selection& sel)
{
	pvlogger::debug() << "db::algo::reduce_operation(...)" << std::endl;

	assert(in_array);

	std::vector<T> partial_results;

	core::algo::__impl::map(
	    sel, in_array.size(), [&](size_t slices_count) { partial_results.resize(slices_count); },
	    [&](size_t pos, size_t len, size_t th_index) {
		    const db::array array_slice = in_array.slice(pos, len);
		    const db::selection sel_slice = sel.slice(pos, len);

		    partial_results[th_index] = std::mem_fn(reduce_op)(array_slice, sel_slice);
		});

	db::array final_array = merge_op(partial_results, in_array.type());

	return std::mem_fn(reduce_op)(final_array, db::selection());
}
}

db::array sum(const db::array& array, const db::selection& sel /*= db::selection()*/)
{
	return __impl::reduce_operation(array, &db::array::sum, sel);
}

db::array average(const db::array& array, const db::selection& sel /*= db::selection()*/)
{
	size_t vsel_count = array.valid_count(sel);
	if (vsel_count) {
		db::array sum_array = sum(array, sel);
		db::array divisor("number_uint64", 1);
		divisor.to_core_array<uint64_t>()[0] = vsel_count;
		return sum_array.divide(divisor);
	} else {
		return {};
	}
}

db::array min(const db::array& array, const db::selection& sel /*= db::selection()*/)
{
	return __impl::reduce_operation(array, &db::array::min, sel);
}

db::array max(const db::array& array, const db::selection& sel /*= db::selection()*/)
{
	return __impl::reduce_operation(array, &db::array::max, sel);
}

db::array minmax(const db::array& array, const db::selection& sel /*= db::selection()*/)
{
	return __impl::reduce_operation(array, &db::array::minmax, sel);
}

} // namespace pvcop
