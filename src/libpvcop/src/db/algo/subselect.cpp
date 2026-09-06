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

#include <algorithm>

namespace pvcop::db::algo
{

db::array to_array(const db::array& array,
                   const std::vector<std::string>& values,
                   std::vector<std::string>* unconvertable_values /*= nullptr*/)
{
	std::vector<db::array> partial_convertions;
	std::vector<std::vector<std::string>> partial_unconvertable_values;

	// parallelize search strings convertion to native representation
	core::algo::__impl::map({}, values.size(),
	                        [&](size_t slices_count) {
		                        partial_convertions.resize(slices_count);
		                        partial_unconvertable_values.resize(slices_count);
		                    },
	                        [&](size_t pos, size_t len, size_t th_index) {
		                        partial_convertions[th_index] = array.to_array(
		                            values.begin() + pos, values.begin() + pos + len,
		                            unconvertable_values ? &partial_unconvertable_values[th_index]
		                                                 : nullptr);
		                    });

	if (unconvertable_values) {
		for (size_t i = 1; i < partial_unconvertable_values.size(); i++) {
			std::move(partial_unconvertable_values[i].begin(),
			          partial_unconvertable_values[i].end(),
			          std::back_inserter(partial_unconvertable_values[0]));
		}
		*unconvertable_values = std::move(partial_unconvertable_values[0]);
	}

	return db::array::concat(partial_convertions);
}

void subselect(const db::array& column,
               const db::array& search_values,
               const db::selection& in_sel,
               db::selection& out_sel)
{
	if (not search_values) {
	    return;
	}

	// parallelize call to subselect primitive
	core::algo::__impl::map(
	    in_sel, column.size(), [&](size_t pos, size_t len, size_t /*th_index*/) {
		    const db::array array_slice = column.slice(pos, len);
		    const db::selection input_sel_slice = in_sel.slice(pos, len);
		    db::selection output_sel_slice = out_sel.slice(pos, len);

		    array_slice.subselect(search_values, input_sel_slice, output_sel_slice);
		});
}

void subselect_if(const db::array& array,
                  const std::vector<std::string>& expr_values,
                  std::function<bool(const std::string& array_value, const std::string& expr_value)>
                      custom_match_f,
                  const db::selection& in_sel,
                  db::selection& out_sel)
{
	pvlogger::debug() << "db::algo::subselect(...)" << std::endl;

	assert(in_sel.size() == out_sel.size() || not in_sel);

	// Compute distinct values
	db::array distinct_array;
	distinct(array, distinct_array, in_sel);

	if (distinct_array.size() == 0) {
		// there is nothing we can do if we don't have any value to filter...
		return;
	}

	// Execute string matching on distinct values and get an array of matching native values
	std::vector<db::array> partial_matching;

	core::algo::__impl::map({}, distinct_array.size(),
	                        [&](size_t slices_count) { partial_matching.resize(slices_count); },
	                        [&](size_t pos, size_t len, size_t th_index) {
		                        db::array array_slice = distinct_array.slice(pos, len);
		                        partial_matching[th_index] = array_slice.to_array_if(
		                            expr_values.begin(), expr_values.end(), custom_match_f);
		                    });

	// Pass the array of matching values to subselect algo
	subselect(array, db::array::concat(partial_matching), in_sel, out_sel);
}

} // namespace pvcop
