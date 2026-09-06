/* * MIT License
 *
 * © ESI Group, 2015
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 *
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 *
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __PVCOP_DB__ALGO_H__
#define __PVCOP_DB__ALGO_H__

#include <pvcop/core/impl/bit.h>
#include <pvcop/db/array.h>
#include <pvcop/db/selection.h>

#include <functional>

namespace pvcop
{

namespace db
{

class array;
class groups;
class extents;

namespace algo
{

db::array sum(const db::array& array, const db::selection& sel = db::selection());

db::array average(const db::array& array, const db::selection& sel = db::selection());

db::array min(const db::array& array, const db::selection& sel = db::selection());

db::array max(const db::array& array, const db::selection& sel = db::selection());

/**
 * Compute min and max value of a given array.
 *
 * @return first element is the min and second is the max.
 */
db::array minmax(const db::array& array, const db::selection& sel = db::selection());

/**
 * Compute the distinct values of a given array as well as the associated histogram
 *
 * @param[in] array_in column on which the group is made
 * @param[out] array_out distinct values of in_array
 * @param[out] histogram count of each distinct values
 * @param[in] sel the selection used to filter the input array
 */
void distinct(const db::array& in_array,
              db::array& out_array,
              db::array& histogram,
              const db::selection& sel = db::selection());

/**
 * Compute the distinct values of a given array without the associated histogram
 *
 * @param[in] array_in column on which the group is made
 * @param[out] array_out distinct values of in_array
 * @param[in] sel the selection used to filter the input array
 */
void distinct(const db::array& in_array,
              db::array& out_array,
              const db::selection& sel = db::selection());

/**
 * Filters an array with a range of value.
 *
 * @param[in] array array on which the filtering is done
 * @param[in] min : minimum possible value
 * @param[in] maximum : maximum possible value
 * @param[in] in_sel input selection
 * @param[out] out_sel resulting selection
 */
void range_select(const db::array& array,
                  const std::string& min,
                  const std::string& max,
                  const db::selection& in_sel,
                  db::selection& out_sel);

/**
 * Convert a list of strings to their native representation
 *
 * @param[in] array array used to compute the native values
 * @param[in] values list of string values to filter
 * @param[out] unconvertable_values values not in valid and invalid domains
 *
 * @return db::array containing the converted values (valid and invalid)
 */
db::array to_array(const db::array& array,
                   const std::vector<std::string>& values,
                   std::vector<std::string>* unconvertable_values = nullptr);

/**
 * Filters an array with a list of values (exact match)
 *
 * @param[in] search_array values on which the filtering is done
 * @param[in] values array of values to filter
 * @param[in] in_sel input selection
 * @param[out] out_sel resulting selection
 * @param[in] invalid_search_array invalid values on which the filtering is done
 */
void subselect(const db::array& search_array,
               const db::array& values,
               const db::selection& in_sel,
               db::selection& out_sel);

/**
 * Filters an array with a binary predicate (multiple match)
 *
 * @param[in] array array on which the filtering is done
 * @param[in] expr_values list of string values to filter
 * @param[in] custom_match_f function tacking 2 strings as parameters (value and expression).
 *            must return true if value match the expression, false otherwise
 * @param[in] in_sel input selection
 * @param[out] out_sel resulting selection
 */
void subselect_if(
    const db::array& array,
    const std::vector<std::string>& expr_values,
    std::function<bool(const std::string& array_value, const std::string& expr_value)>,
    const db::selection& in_sel,
    db::selection& out_sel);

/**
 * Compute the sum on column2 of each unique value of column1
 *
 * This is equivalent to the following SQL query :
 * `SELECT column1, SUM(column2) FROM collection GROUP BY column1;`
 *
 * @param[in] array_in_1 column on which the group is made
 * @param[in] array_in_2 values to sum for each value of array_in_1
 * @param[out] array_out_1 distinct values of array_in_1
 * @param[out] array_out_2 sum of array_in_2 for each distinct value of array_in_1
 * @param[in] sel the selection used to filter the input arrays
 */
void sum_by(const db::array& in_array1,
            const db::array& in_array2,
            db::array& out_array1,
            db::array& out_array2,
            const db::selection& sel = db::selection());

void count_by(const db::array& in_array1,
              const db::array& in_array2,
              db::array& out_array1,
              db::array& out_array2,
              const db::selection& sel = db::selection());

void min_by(const db::array& in_array1,
            const db::array& in_array2,
            db::array& out_array1,
            db::array& out_array2,
            const db::selection& sel = db::selection());

void max_by(const db::array& in_array1,
            const db::array& in_array2,
            db::array& out_array1,
            db::array& out_array2,
            const db::selection& sel = db::selection());

void average_by(const db::array& in_array1,
                const db::array& in_array2,
                db::array& out_array1,
                db::array& out_array2,
                const db::selection& sel = db::selection());

/**
 * Get the intermediary results of any of the xxx_by operation
 * Intermediary results are the list of values that lead to the
 * obtention of the final results, eg: the list of counted (or summed, etc)
 * values on the second column for for a specific value on the first column.
 *
 * @param[in]  array_in_1 first column of the xxx_by operation
 * @param[in]  array_in_2 second column of the xxx_by operation
 * @param[in]  value value to filter from the first column
 * @param[out] out_array1 distinct values of the second column filtered by `value`
 * @param[out] histogram count of each values of `out_array1`
 * @param[in]  in_sel optional input selection
 *
 * @throw db::exception::unable_to_convert_input_error
 */
void op_by_details(const db::array& in_array1,
                   const db::array& in_array2,
                   const std::string& value,
                   db::array& out_array1,
                   db::array& histogram,
                   const db::selection& in_sel = db::selection());

} // namespace pvcop::db::algo

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB__ALGO_H__
