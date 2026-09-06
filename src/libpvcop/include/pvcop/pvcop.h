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

#ifndef __PVCOP_H__
#define __PVCOP_H__

#include <pvcop/core/array.h>
#include <pvcop/core/memarray.h>
#include <pvcop/db/array.h>
#include <pvcop/db/algo.h>

#include <pvcop/types/formatter/formatter_interface.h>
#include <pvcop/types/impl/formatter_factory.h>

/**
 * @mainpage
 *
 * This functions library provides a framework to process and manipulate huge
 * volumes of data using a column oriented database approach.
 * It only provides structures and functions to work on data, not on their
 * meanings. Supported types are also simple: scalars (signed/unsigned
 * integers and floating point numbers) and strings.
 *
 * This functions library is splitted into 2 main namespaces: @b pvcop::core
 * and @b pvcop::db.
 *
 * @a pvcop::core provides low-level containers to store data and generic
 * functions to travers those containers (see pvcop::core::array).
 *
 * @a pvcop::db provides the database logic which is only based on the application
 * of operators on array to produce new array. Contrary to the class
 * pvcop::core::array where the type is explicit, the class pvcop::db::array
 * uses an implicit typing approach to provide a clean and lean API (see
 * pvcop::db::array for further informations).
 *
 * The different columns composing a database are assembled in a collection
 * (see pvcop::db::collection) which is an ordered set of typed columns. The
 * term @a column is only used to refer to the column oriented structure of
 * @b pvcop; a column is just an other word for pvcop::db:array. A collection
 * has also a format to describe the number and the types of its columns.
 *
 * @note As @b pvcop is designed to process data, removing or inserting
 * records is a non-sense.
 *
 * However, a pvcop::db::collection is used to process data or change their
 * format but it does not provides append/add functionality. The data set
 * expansion process is done through an other class: pvcop::db::collector.
 *
 * @note This functions library is written using C++11 and thus requires a
 * relatively recent C++ compiler. The following compilers are supported:
 * - GCC 4.8 and above
 * - clang 3.4 and above (clang 3.4 fails to compile with C++ headers of GCC 4.9, clang 3.5 works)
 */

namespace pvcop
{
/**
 * Register a formatter
 */
template <typename Formatter>
bool register_type()
{
	return pvcop::types::__impl::formatter_factory::get().register_type<Formatter>();
}

/**
 * Register a formatter template on all the supported types
 */
template <template <typename> class Formatter>
void register_type()
{
	pvcop::types::__impl::formatter_factory::get().register_type<Formatter>();
}

/**
 * Register a formatter template on all the supplied types
 */
template <template <typename> class Formatter, typename Type, typename... Types>
void register_type()
{
	pvcop::types::__impl::formatter_factory::get().register_type<Formatter, Type, Types...>();
}

} // namespace pvcop

#endif // __PVCOP_H__
