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


#ifndef PVCOP_TYPES_IMPL_NUMBER_TO_STRING__H
#define PVCOP_TYPES_IMPL_NUMBER_TO_STRING__H

#include <pvcop/db/types.h>

#include <cassert>
#include <charconv>
#include <cstdio>
#include <system_error>
#include <type_traits>

namespace pvcop
{

namespace types
{

namespace __impl
{

/**
 * @return the written length, or a negative value if the buffer was too small, as the
 *         formatter interface expects.
 */
template <typename T, typename... Format>
inline int write(char* str, const size_t str_len, const T& value, Format... format)
{
	if (str_len == 0) {
		return -1;
	}

	// to_chars writes no terminating NUL, unlike the snprintf it replaces, and callers
	// do read the buffer as a C string: keep one byte for it.
	const std::to_chars_result written = std::to_chars(str, str + str_len - 1, value, format...);
	if (written.ec != std::errc()) {
		return -1;
	}
	*written.ptr = '\0';

	return static_cast<int>(written.ptr - str);
}

template <typename T, class = typename std::enable_if<std::is_unsigned<T>::value>::type>
inline uint64_t cast(T t)
{
	return (uint64_t)t;
}

template <typename T, class = typename std::enable_if<std::is_signed<T>::value>::type>
inline int64_t cast(T t)
{
	return (int64_t)t;
}

template <typename T>
inline int number_to_string(char* str,
                            const size_t str_len,
                            const char* parameters,
                            size_t base,
                            size_t float_precision,
                            char float_mode,
                            const T& value)
{
	return number_to_string(str, str_len, parameters, base, float_precision, float_mode,
	                        cast(value));
}

/**
 * Integers, in the base the column format asks for. to_chars writes no base prefix, so
 * the "0x" and "0" of the hexadecimal and octal notations are laid down by hand, as the
 * printf formats these bases stand for ("0x%x", "%#o") produce them. Any other base
 * falls back to the format string itself.
 */
template <typename T>
inline int integer_to_string(char* str, const size_t str_len, const char* parameters, size_t base,
                             const T& value)
{
	switch (base) {
	case 10:
		return write(str, str_len, value, 10);
	case 16: {
		if (str_len < 2) {
			return -1;
		}
		str[0] = '0';
		str[1] = 'x';
		const int written = write(str + 2, str_len - 2, value, 16);
		return written < 0 ? written : written + 2;
	}
	case 8: {
		// A leading zero is the octal prefix, and zero itself already reads as one
		const size_t prefix = value != 0;
		if (str_len < prefix) {
			return -1;
		}
		str[0] = '0';
		const int written = write(str + prefix, str_len - prefix, value, 8);
		return written < 0 ? written : written + static_cast<int>(prefix);
	}
	default:
		return snprintf(str, str_len, parameters, value);
	}
}

template <>
inline int number_to_string(char* str,
                            const size_t str_len,
                            const char* parameters,
                            size_t base,
                            size_t /*float_precision*/,
                            char /*float_mode*/,
                            const int64_t& value)
{
	return integer_to_string(str, str_len, parameters, base, value);
}

template <>
inline int number_to_string(char* str,
                            const size_t str_len,
                            const char* parameters,
                            size_t base,
                            size_t /*float_precision*/,
                            char /*float_mode*/,
                            const uint64_t& value)
{
	return integer_to_string(str, str_len, parameters, base, value);
}

/**
 * Floating point values, in the notation the column format asks for: 'e' scientific,
 * 'f' fixed, 'g' general, which map one to one onto std::chars_format.
 */
template <typename T>
inline int float_to_string(char* str, const size_t str_len, size_t float_precision,
                           char float_mode, const T& value)
{
	const int precision = static_cast<int>(float_precision);

#ifdef __cpp_lib_to_chars
	switch (float_mode) {
	case 'e':
		return write(str, str_len, value, std::chars_format::scientific, precision);
	case 'f':
		return write(str, str_len, value, std::chars_format::fixed, precision);
	case 'g':
		return write(str, str_len, value, std::chars_format::general, precision);
	default:
		assert(false);
		return {};
	}
#else
	// Where to_chars has no floating point overload to call (see string_to_number.h),
	// the printf conversions write the very notations chars_format stands for, and the
	// terminating NUL of their own accord.
	const char* format;
	switch (float_mode) {
	case 'e':
		format = "%.*e";
		break;
	case 'f':
		format = "%.*f";
		break;
	case 'g':
		format = "%.*g";
		break;
	default:
		assert(false);
		return {};
	}

	const int written = snprintf(str, str_len, format, precision, static_cast<double>(value));

	return (written < 0 or static_cast<size_t>(written) >= str_len) ? -1 : written;
#endif
}

template <>
inline int number_to_string(char* str,
                            const size_t str_len,
                            const char* /*parameters*/,
                            size_t /*base*/,
                            size_t float_precision,
                            char float_mode,
                            const double& value)
{
	return float_to_string(str, str_len, float_precision, float_mode, value);
}

template <>
inline int number_to_string(char* str,
                            const size_t str_len,
                            const char* /*parameters*/,
                            size_t /*base*/,
                            size_t float_precision,
                            char float_mode,
                            const float& value)
{
	return float_to_string(str, str_len, float_precision, float_mode, value);
}

template <>
inline int number_to_string(char* /*str*/,
                            const size_t /*str_len*/,
                            const char* /*parameters*/,
                            size_t /*base*/,
                            size_t /*float_precision*/,
                            char /*float_mode*/,
                            const pvcop::db::uint128_t& /*value*/)
{
	assert(false && "not supported yet");
	return 0;
}

} // namespace pvcop::types::__impl

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_IMPL_NUMBER_TO_STRING__H
