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

#ifndef PVCOP_TYPES_IMPL_NUMBER_ADAPTER_H
#define PVCOP_TYPES_IMPL_NUMBER_ADAPTER_H

#include <cmath>
#include <cstddef>

namespace pvcop
{

namespace types
{

namespace __impl
{

/**
 * This class aims to manipulate quantities (units and magnitudes)
 */
class number_adapter
{
  public:
	enum prefix_type { PREFIX_NONE = 0, PREFIX_SI, PREFIX_BIN };

  public:
	/**
	 * Default constructor
	 */
	number_adapter();

	/**
	 * Destructor
	 */
	~number_adapter();

	/**
	 * Initialize a number_adapter
	 *
	 * the use of prefix is determined by parsing the format string to find
	 * a second format pattern.
	 *
	 * @param format the used format string
	 * @param type the prefix type to use
	 * @param use_short_prefix to tell if prefixes must be shortened or not
	 */
	void init(const char* format, const prefix_type type, bool use_short_prefix);

	/**
	 * Checks for prefixes use
	 *
	 * @return true if a prefixe can be used; false otherwise
	 */
	inline bool use_prefix() const { return (_prefixes != nullptr); }

	/**
	 * Adapts and print a value into a string
	 *
	 * @param str a null-terminated string
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return the written size or a negative value to report an error
	 */
	template <typename T>
	int convert_to_string(char* str, const size_t str_len, const char* format, const T& value) const
	{
		if (_prefixes) {
			int id;
			T val = apply_prefix<T>(value, id);
			return snprintf(str, str_len, format, val, _prefixes[id]);
		} else {
			return snprintf(str, str_len, format, value);
		}
	}

  private:
	/**
	 * Determines if a value need to be represented using a
	 * order prefix or not.
	 *
	 * @param value the value to estimate
	 * @param[out] id a value identifying the prefix to use
	 *
	 * @return the (eventually) scaled value
	 */
	template <typename T>
	inline T apply_prefix(const T& value, int& id) const
	{
		if (_prefix_type == PREFIX_NONE) {
			id = 0;
			return value;
		} else if (_prefix_type == PREFIX_SI) {
			id = log2(value) / log2(1000);
			return value / pow(1000., id);
		} else {
			id = log2(value) / log2(1024);
			return value / pow(1024., id);
		}
	}

  private:
	prefix_type _prefix_type;
	const char** _prefixes;
};

} // namespace pvcop::types::__impl

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_IMPL_NUMBER_ADAPTER_H
