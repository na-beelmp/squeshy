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

#ifndef PVCOP_DB_MAP_REDUCE
#define PVCOP_DB_MAP_REDUCE

#include <pvcop/core/impl/map_reduce.h>

#include <pvcop/db/array.h>
#include <pvcop/db/selection.h>

#include <tuple>
#include <utility>

namespace pvcop
{

namespace db
{

namespace algo
{

namespace __impl
{

template <typename T, std::size_t N>
using results_t = std::array<T, N>;

/**
 * Perform map reduce computation.
 * Argument formating is done, we can start computation.
 *
 * @param ins : inputs arrays (r-value to avoir db::array copy)
 * @param out : outputs arrays (r-value to avoir db::array copy)
 * @param map : Map function operation to perform
 * @param reduce : Reduce function operation to perform
 * @param sel : Selection applied to inputs arrays.
 * @param sequences : Utility to iterate over ins and out at compile time.
 *
 * @note I... iterate over inputs array, O... over outputs arrays
 */
template <class Map_f, class Reduce_f, class... Ins, class... Outs, size_t... I, size_t... O>
void map_reduce(std::tuple<Ins const&...>&& ins,
                std::tuple<Outs&...>&& out,
                Map_f map,
                Reduce_f reduce,
                const db::selection& sel,
                std::index_sequence<I...>,
                std::index_sequence<O...>)
{
	pvlogger::debug() << "db::algo::map_reduce(...)" << std::endl;

#ifndef NDEBUG
	// Check every input array have the same size
	int _[] __attribute((unused)) = {
	    (assert(std::get<0>(ins).size() == std::get<I>(ins).size()), 1)...};
#endif

	const size_t input_size = std::get<0>(ins).size(); // Size of inputs arrays.

	// Save partial result for each slice.
	using partial_result_t = results_t<db::array, sizeof...(Outs)>;
	using partial_results_t = std::vector<partial_result_t>;
	partial_results_t partial_results;

	core::algo::__impl::map_reduce(
	    sel, input_size, [&](size_t slices_count) { partial_results.resize(slices_count); },
	    [&](size_t pos, size_t len, size_t th_index) {
		    // Apply map function for each slice.
		    map(std::get<I>(ins).slice(pos, len)..., std::get<O>(partial_results[th_index])...,
		        sel.slice(pos, len));
		},
	    [&]() {
		    // Concatenate every partial result
		    using tmp_partial_results_t = results_t<std::vector<db::array>, sizeof...(Outs)>;
		    tmp_partial_results_t tmp_partial_results;

		    for (size_t i = 0; i < partial_results.size(); i++) {
			    int _[] __attribute((unused)) = {
			        (std::get<O>(tmp_partial_results)
			             .emplace_back(std::move(std::get<O>(partial_results[i]))),
			         1)...};
		    }

		    partial_result_t final_array;
		    int _[] __attribute((unused)) = {
		        (std::get<O>(final_array) = db::array::concat(std::get<O>(tmp_partial_results)),
		         1)...};

		    // Apply partial result on accumulation
		    reduce(std::get<O>(final_array)..., std::get<O>(out)...);
		});
}

/**
 * Utility function to split array in "In arrays" and "Out arrays" tuple.
 *
 * Tuples do not use make_tuple as we do not want any copy.
 * arr is a r-value tuple as it is created by the caller (us) and we don't want to copy content.
 */
template <class Map_f, class Reduce_f, size_t... In, size_t... Out, class... T>
void map_reduce(std::index_sequence<In...> i_s,
                std::index_sequence<Out...> o_s,
                Map_f map,
                Reduce_f reduce,
                const db::selection& sel,
                std::tuple<T...>&& arr)
{
	map_reduce(
	    std::tuple<typename std::tuple_element<In, std::tuple<T...>>::type...>(
	        std::get<In>(arr)...),
	    std::tuple<typename std::tuple_element<sizeof...(In) + Out, std::tuple<T...>>::type...>(
	        std::get<sizeof...(In) + Out>(arr)...),
	    map, reduce, sel, i_s, o_s);
}
}

/**
 * Function call to perform parallel map reduce computation.
 *
 * It will take `In` arrays as inputs, `Out` arrays as output.
 * Inputs arrays will be sliced. For each slice, map function will be apply
 * and result will be save in a new created array.
 * These new created array will be concatenate and reduce will be apply on
 * this new big array. Result will be save in out arrays.
 */
template <size_t In, size_t Out, class Map_f, class Reduce_f, class... T>
void map_reduce(Map_f map, Reduce_f reduce, const db::selection& sel, T&&... arr)
{
	__impl::map_reduce(decltype(std::make_index_sequence<In>())(),
	                   decltype(std::make_index_sequence<Out>())(), map, reduce, sel,
	                   std::tuple<T&&...>(arr...));
}
}
}
}

#endif
