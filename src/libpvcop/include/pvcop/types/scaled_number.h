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

#ifndef PVCOP_TYPES_FORMATTER_SCALED_NUMBER_H
#define PVCOP_TYPES_FORMATTER_SCALED_NUMBER_H

#include <pvcop/types/number.h>

namespace pvcop
{

namespace types
{

/**
 * This class defines a formatter for number where values are
 * divided by a constant.
 *
 * This class permits to operated scale changes like displaying send bytes
 * quantities in Kio or Mio instead of octets or distances expressed in
 * inch but displayed in kilimeters.
 *
 * To do so (for the first example) create the following formatter :
 * formatter_scaled_number("%lu Kio", 1024);
 */
template <typename T>
class formatter_scaled_number : public formatter_number<T>
{
  public:
	/**
	 * Default constructor
	 *
	 * @param format the format string for values
	 * @param factor the constant by which all values will be divided
	 */
	formatter_scaled_number(const char* format, const T factor)
	    : formatter_number<T>(format), _factor(factor)
	{
	}

  protected:
	/**
	 * Overrides pvcop::types::formatter_number::convert_to_string
	 *
	 * @param str a null-terminated string
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return the written size or a negative value to report an error
	 */
	int convert_to_string(char* str, const size_t str_len, const T& value) const override
	{
		return formatter_number<T>::convert_to_string(str, str_len, value / _factor);
	}

  private:
	T _factor;
};

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_FORMATTER_SCALED_NUMBER_H
