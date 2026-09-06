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
#include <pvcop/db/map_reduce.h>
#include <pvcop/db/array.h>
#include <pvcop/core/impl/map_reduce.h>
#include <pvcop/core/memarray.h>

#include <pvlogger.h>

#include <algorithm>
#include <iostream>

namespace pvcop::db::algo
{

namespace __impl
{

bool assert_same_size(const db::array& in_array1, const db::array& in_array2, const char* func)
{
	bool same_size = in_array1.size() == in_array2.size();

	if (!same_size) {
		pvlogger::error() << func << ": input arrays have different size." << std::endl;
	}

	return same_size;
}

} // namespace pvcop::db::algo::__impl

using group_by_operation_t = db::array (db::array::*)(const db::groups&,
                                                      const db::extents&,
                                                      const db::selection&) const;

/**
 * Compute in_array1 distinct value in out_array1 and save result of operation
 * apply on in_array2 with in_array 1 grouping in out_array2.
 *
 * @param operation1 : map operation to apply
 * @param operation2 : reduce operation to apply
 */
static void group_by_operation(const db::array& in_array1,
                               const db::array& in_array2,
                               db::array& out_array1,
                               db::array& out_array2,
                               group_by_operation_t operation1,
                               group_by_operation_t operation2,
                               const db::selection& sel /*= db::selection()*/
                               )
{
	pvlogger::trace() << "db::algo::__impl::group_by_operation(...)" << std::endl;

	auto map = [&](const db::array& in_array1, const db::array& in_array2, db::array& out_array1,
	               db::array& out_array2, const db::selection& sel) {
		// group
		db::groups groups;
		db::extents extents;
		in_array1.group(groups, extents, sel);

		// right fetch join
		out_array1 = in_array1.join(extents);

		// operation
		out_array2 = (in_array2.*operation1)(groups, extents, sel);
	};

	auto reduce = [&](const db::array& in_array1, const db::array& in_array2, db::array& out_array1,
	                  db::array& out_array2) {
		// group
		db::groups groups;
		db::extents extents;
		in_array1.group(groups, extents);

		// right fetch join
		out_array1 = in_array1.join(extents);

		// operation
		out_array2 = (in_array2.*operation2)(groups, extents, {});
	};

	db::algo::map_reduce<2, 2>(map, reduce, sel, in_array1, in_array2, out_array1, out_array2);
}

static void group_by_operation(const db::array& in_array1,
                               const db::array& in_array2,
                               db::array& out_array1,
                               db::array& out_array2,
                               group_by_operation_t operation,
                               const db::selection& sel /*= db::selection()*/
                               )
{
	group_by_operation(in_array1, in_array2, out_array1, out_array2, operation, operation, sel);
}

void distinct(const db::array& in_array,
              db::array& out_array,
              const db::selection& sel /*= db::selection()*/)
{
	pvlogger::debug() << "db::algo::distinct(...)" << std::endl;

	auto map = [&](const db::array& in_array, db::array& out_array, const db::selection& sel) {
		// group
		db::groups groups;
		db::extents extents;
		in_array.group(groups, extents, sel);

		// right fetch join
		out_array = in_array.join(extents);
	};

	auto reduce = [&](const db::array& in_array, db::array& out_array) {
		// group
		db::groups groups;
		db::extents extents;
		in_array.group(groups, extents);

		// right fetch join
		out_array = in_array.join(extents);
	};

	db::algo::map_reduce<1, 1>(map, reduce, sel, in_array, out_array);
}

void distinct(const db::array& in_array,
              db::array& out_array,
              db::array& histogram,
              const db::selection& sel /*= db::selection()*/)
{
	pvlogger::debug() << "db::algo::distinct(...)" << std::endl;

	group_by_operation(in_array, in_array, out_array, histogram, &db::array::group_count,
	                   &db::array::group_sum, sel);
}

void sum_by(const db::array& in_array1,
            const db::array& in_array2,
            db::array& out_array1,
            db::array& out_array2,
            const db::selection& sel /*= db::selection()*/
            )
{
	pvlogger::debug() << "db::algo::sum_by(...)" << std::endl;

	if (!__impl::assert_same_size(in_array1, in_array2, __func__)) {
		return;
	}

	group_by_operation(in_array1, in_array2, out_array1, out_array2, &db::array::group_sum, sel);
}

void count_by(const db::array& in_array1,
              const db::array& in_array2,
              db::array& out_array1,
              db::array& out_array2,
              const db::selection& sel /*= db::selection()*/
              )
{
	pvlogger::debug() << "db::algo::count_by(...)" << std::endl;

	if (!__impl::assert_same_size(in_array1, in_array2, __func__)) {
		return;
	}

	// group
	db::groups groups;
	db::extents extents;
	in_array1.group(groups, extents, sel);

	// right fetch join
	out_array1 = in_array1.join(extents);

	// distinct count
	out_array2 = in_array2.group_distinct_count(groups, extents, sel);
}

void min_by(const db::array& in_array1,
            const db::array& in_array2,
            db::array& out_array1,
            db::array& out_array2,
            const db::selection& sel /*= db::selection()*/
            )
{
	pvlogger::debug() << "db::algo::min_by(...)" << std::endl;

	if (!__impl::assert_same_size(in_array1, in_array2, __func__)) {
		return;
	}

	group_by_operation(in_array1, in_array2, out_array1, out_array2, &db::array::group_min, sel);
}

void max_by(const db::array& in_array1,
            const db::array& in_array2,
            db::array& out_array1,
            db::array& out_array2,
            const db::selection& sel /*= db::selection()*/
            )
{
	pvlogger::debug() << "db::algo::max_by(...)" << std::endl;

	if (!__impl::assert_same_size(in_array1, in_array2, __func__)) {
		return;
	}

	group_by_operation(in_array1, in_array2, out_array1, out_array2, &db::array::group_max, sel);
}

void average_by(const db::array& in_array1,
                const db::array& in_array2,
                db::array& out_array1,
                db::array& out_array2,
                const db::selection& sel /*= db::selection()*/
                )
{
	pvlogger::debug() << "db::algo::average_by(...)" << std::endl;

	if (!__impl::assert_same_size(in_array1, in_array2, __func__)) {
		return;
	}

	// this array is used to store intermediary counts allowing to divide values at the end
	db::array count_array;

	auto map = [&](const db::array& in_array1, const db::array& in_array2, db::array& out_array1,
	               db::array& out_array2, db::array& out_array3, const db::selection& sel) {
		// group
		db::groups groups;
		db::extents extents;
		in_array1.group(groups, extents, sel);

		// right fetch join
		out_array1 = in_array1.join(extents);

		// operation
		out_array2 = in_array2.group_sum(groups, extents, sel);
		out_array3 = in_array2.group_count(groups, extents, sel);
	};

	auto reduce = [&](const db::array& in_array1, const db::array& in_array2,
	                  const db::array& in_array3, db::array& out_array1, db::array& out_array2,
	                  db::array& out_array3) {
		// group
		db::groups groups;
		db::extents extents;
		in_array1.group(groups, extents);

		// right fetch join
		out_array1 = in_array1.join(extents);

		// operation
		out_array2 = in_array2.group_sum(groups, extents);
		out_array3 = in_array3.group_sum(groups, extents);
		out_array2 = out_array2.divide(out_array3);
	};

	db::algo::map_reduce<2, 3>(map, reduce, sel, in_array1, in_array2, out_array1, out_array2,
	                           count_array);
}

void op_by_details(const db::array& in_array1,
                   const db::array& in_array2,
                   const std::string& value,
                   db::array& out_array1,
                   db::array& out_array2,
                   const db::selection& in_sel /*= db::selection()*/)
{
	pvlogger::debug() << "db::algo::op_by_details(...)" << std::endl;

	std::vector<db::array> partial_results;

	const db::array& converted_array = in_array1.to_array(std::vector<std::string>({value}));

	core::algo::__impl::map(
	    in_sel, in_array1.size(),
	    [&](const size_t slices_count) { partial_results.resize(slices_count); },
	    [&](size_t pos, size_t len, size_t th_index) {
		    const db::array in_array1_slice = in_array1.slice(pos, len);
		    const db::array in_array2_slice = in_array2.slice(pos, len);
		    const db::selection in_sel_slice = in_sel.slice(pos, len);

		    // for each slice : filter the value on the first column then join on the second column
		    pvcop::core::memarray<bool> out_sel_slice(in_sel_slice.size());
		    in_array1_slice.subselect(converted_array, in_sel_slice, out_sel_slice);
		    partial_results[th_index] = in_array2_slice.join(out_sel_slice);
		});

	// merge results
	db::array final_array = db::array::concat(partial_results);

	// only keep distinct values
	distinct(final_array, out_array1, out_array2);

	assert(out_array1.size() == out_array2.size());
}

} // namespace pvcop
