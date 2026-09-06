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

#ifndef PVCOP_TYPES_FORMATTER_ADAPTATIVE_NUMBER_H
#define PVCOP_TYPES_FORMATTER_ADAPTATIVE_NUMBER_H

#include <pvcop/types/number.h>
#include <pvcop/types/impl/number_adapter.h>

namespace pvcop
{

namespace types
{

using unit_prefix_type = __impl::number_adapter::prefix_type;

/**
 * This class defines a formatter for scalar values.
 */
template <typename T>
class formatter_adaptative_number : public formatter_number<T>
{
  public:
	static constexpr char* no_unit = nullptr;

  public:
	/**
	 * Default constructor
	 *
	 * The underlying adapter parse the format string to find if a second
	 * format pattern (which must be "%s") is present or not. It permits
	 * some internal optimization.
	 *
	 * @param format the format string
	 * @param prefix_type the unit prefix (default to no computed prefix)
	 * @param use_short_prefix a boolean to tell if the short prefix version
	 * or the long one must be used (default to long prefix)
	 */
	formatter_adaptative_number(const char* format,
	                            unit_prefix_type prefix_type = unit_prefix_type::PREFIX_NONE,
	                            bool use_short_prefix = false)
	    : formatter_number<T>(format)
	{
		_adapter.init(format, prefix_type, use_short_prefix);
	}

	/**
	 * Destructor
	 */
	~formatter_adaptative_number() {}

  protected:
	/**
	 * Overrides pvcop::types::formatter::convert_to_string
	 *
	 * @param str a null-terminated string
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return the written size or a negative value to report an error
	 */
	int convert_to_string(char* str, const size_t str_len, const T& value) const override
	{
		return _adapter.convert_to_string<T>(str, str_len, this->_parameters.c_str(), value);
	}

  private:
  protected:
	mutable __impl::number_adapter _adapter;
};

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_FORMATTER_ADAPTATIVE_NUMBER_H
