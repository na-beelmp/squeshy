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

#ifndef __PVCOP_TYPES_FORMATTER_DATETIME_H__
#define __PVCOP_TYPES_FORMATTER_DATETIME_H__

#include <pvcop/types/formatter/formatter.h>

#include <stdint.h>

namespace pvcop
{

namespace types
{

/**
 * This class defines a formatter for datetime values.
 */
class formatter_datetime : public formatter<uint64_t>
{
  public:
	/**
	 * Default constructor
	 *
	 * @param name the formatter name
	 */
	formatter_datetime(const char* parameters);

  public:
	std::string name() const override { return "datetime"; }
	void set_parameters(const char* parameters) override
	{
		formatter<uint64_t>::set_parameters(parameters);
		_is_year_specified = strstr(parameters, "%y") or strstr(parameters, "%Y");
	}

  public:
	static time_t mktime(struct tm* tm);
	static tm* gmtime_r(const uint64_t* timep, tm* result);

  protected:
	/**
	 * Overrides pvcop::types::formatter::from_string
	 *
	 * @param str the buffer to read from
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return true on success; false otherwise
	 */
	bool
	convert_from_string(const char* str, reference value, bool* pass_autodetect) const override;

	/**
	 * Overrides pvcop::types::formatter::to_string
	 *
	 * @param str a buffer to write to
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return the written size or a negative value to report an error
	 */
	int convert_to_string(char* str, const size_t str_len, const type& value) const override;

  private:
	bool _is_year_specified;
};
}
}

#endif // __PVCOP_TYPES_FORMATTER_DATETIME_H__
