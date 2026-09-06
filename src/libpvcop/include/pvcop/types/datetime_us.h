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

#ifndef __PVCOP_TYPES_FORMATTER_DATETIME_US_H__
#define __PVCOP_TYPES_FORMATTER_DATETIME_US_H__

#include <pvcop/types/formatter/formatter.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace pvcop
{

namespace types
{

/**
 * This class defines a formatter for datetime values
 * with precision up to the microsecond (using boost).
 *
 * Internal representation is not a microsecond timestamps retrived
 * using boost::posix_time::time_duration::total_microseconds
 * since the reverse convertion to boost::posix_time::ptime is very costly.
 * Instead, we use a reinterpret_cast to convert ptime <=> uint64_t which
 * also keeps the timestamps sortable.
 */
class formatter_datetime_us : public formatter<boost::posix_time::ptime>
{
  public:
	union cal {
		boost::posix_time::ptime as_time;
		uint64_t as_value;

		cal(decltype(as_value) a) : as_value(a) {}
		cal(decltype(as_time) b) : as_time(b) {}
	};

  public:
	/**
	 * Default constructor
	 *
	 * @param name the formatter name
	 */
	formatter_datetime_us(const char* parameters);

  public:
	std::string name() const override { return "datetime_us"; }
	void set_parameters(const char* parameters) override;

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
	/**
	 * Get the representation of a string without alphanumeric characters
	 *
	 * This is used to harness the checks made by boost when parsing the string by
	 * comparing the structure of the time parameters with the structure of the time string
	 * to discard unmatching strings.
	 *
	 * @param str the time string
	 * @param the provided buffer to store the string structure to avoid uneeded allocations
	 */
	const char* extract_struct(const char* str, char* buffer) const;

  private:
	boost::posix_time::time_input_facet* _input_facet;
	boost::posix_time::time_facet* _output_facet;
	std::locale _input_locale_facet;  //!< locale object used for parsing (input)
	std::locale _output_locale_facet; //!< locale object used for formatting (output)
	bool _is_year_specified;
	char _params_struct[MAX_STRING_LENGTH];
};

} // namespace pvcop::types

} // namespace pvcop

DEFINE_HASH_FUNCTION(boost::posix_time::ptime);

#endif // __PVCOP_TYPES_FORMATTER_DATETIME_US_H__
