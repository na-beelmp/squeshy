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

#ifndef __PVCOP_DB_EXCEPTIONS_PARTIALLY_CONVERTED_ERROR_H__
#define __PVCOP_DB_EXCEPTIONS_PARTIALLY_CONVERTED_ERROR_H__

#include <string>
#include <vector>
#include <memory>

namespace pvcop
{

namespace db
{

class array;

namespace exception
{

/**
 * Exception thrown when a list of strings fails to be
 * completely converted to its native representation.
 * This is uselful to let the user chose whenever he wishes
 * to continue with an incomplete array or abort the operation.
 *
 * incomplete_array() : the converted array minus values that failed to be converted
 * bad_values() : list of values that failed to be converted
 */
struct partially_converted_error : public std::exception {
  public:
	using strings_t = std::vector<std::string>;

  public:
	partially_converted_error(db::array array, strings_t values);

	db::array& incomplete_array() { return *_incomplete_array; }

	const db::array& incomplete_array() const { return *_incomplete_array; }

	strings_t& bad_values() { return _bad_values; }

	const strings_t& bad_values() const { return _bad_values; }

  private:
	std::unique_ptr<db::array> _incomplete_array;
	strings_t _bad_values;
};

} // pvcop::db::exception

} // pvcop::db

} // pvcop

#endif // __PVCOP_DB_EXCEPTIONS_PARTIALLY_CONVERTED_ERROR_H__
