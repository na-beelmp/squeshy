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

#ifndef PVCOP_TYPES_IMPL_STRING_TO_NUMBER_H
#define PVCOP_TYPES_IMPL_STRING_TO_NUMBER_H

#include <pvcop/db/types.h>

#include <cassert>
#include <cerrno>
#include <charconv>
#include <cstdlib>
#include <string.h>
#include <system_error>
#include <type_traits>

namespace pvcop
{

namespace types
{

namespace __impl
{

/**
 * std::from_chars is stricter than the strtol/strtod family the callers were written
 * against: it rejects leading whitespace, an explicit plus sign, and the 0x prefix of
 * hexadecimal values. Skip those so that any input accepted before keeps being accepted.
 *
 * A sign in front of a 0x prefix is not handled, as hexadecimal and octal bases are only
 * selected by an explicitly unsigned column format ("0x%x", "%#o").
 */
inline const char* skip_unsupported_prefix(const char* str, const size_t base)
{
	while (*str == ' ' or (*str >= '\t' and *str <= '\r')) {
		str++;
	}
	if (*str == '+') {
		str++;
	}
	if (base == 16 and str[0] == '0' and (str[1] == 'x' or str[1] == 'X')) {
		str += 2;
	}
	return str;
}

/**
 * Parse the whole string, leaving nothing behind: a trailing character means the value
 * is not of that type, which is what drives the type autodetection.
 */
template <typename T, typename... Format>
inline T parse(const char* str, const char* first, bool& res, Format... format)
{
	T value{};
	const char* const last = str + strlen(str);
	const std::from_chars_result parsed = std::from_chars(first, last, value, format...);

	res = parsed.ec == std::errc() and parsed.ptr == last;

	return res ? value : T{};
}

template <typename T>
inline T string_to_number(const char*, const size_t, bool& res, bool* /*pass_autodetect*/);

template <>
inline string_index_t
string_to_number<string_index_t>(const char*, const size_t, bool& res, bool* /*pass_autodetect*/)
{
	assert(false && "should not be used");
	res = false;
	return 0;
}

template <>
inline bool
string_to_number<bool>(const char* str, const size_t, bool& res, bool* /*pass_autodetect*/)
{
	res = true;

	if ((strncmp(str, "0", 1) == 0 or strncasecmp(str, "false", 5) == 0)) {
		return false;
	}
	if ((strncmp(str, "1", 1) == 0 or strncasecmp(str, "true", 4) == 0)) {
		return true;
	}

	res = false;
	return false;
}

#ifndef __cpp_lib_to_chars

/**
 * The floating point conversions of the libc++ the macOS SDK ships are unusable: it has
 * no from_chars overload for them at all, and its to_chars is marked as introduced in
 * macOS 13.3, way above the deployment target of the build. libc++ only defines
 * __cpp_lib_to_chars once both are there, which is what tells such a standard library
 * apart. Fall back to strtof/strtod, which is what this used to call everywhere, only
 * turning down what from_chars turns down as well: the hexadecimal notation it has no
 * format for.
 */
template <typename T>
inline T parse_float(const char* str, bool& res)
{
	const char* const first = skip_unsupported_prefix(str, 10);
	if (first[0] == '0' and (first[1] == 'x' or first[1] == 'X')) {
		res = false;
		return T{};
	}

	char* end = nullptr;
	errno = 0;
	T value;
	if constexpr (std::is_same<T, float>::value) {
		value = std::strtof(first, &end);
	} else {
		value = std::strtod(first, &end);
	}

	res = end != first and *end == '\0' and errno == 0;

	return res ? value : T{};
}

#endif // __cpp_lib_to_chars

/**
 * Floating point values, whatever the column format: from_chars only knows about the
 * value itself, and "general" covers both the fixed and the scientific notations.
 */
template <typename T>
inline T string_to_float(const char* str, bool& res, bool* pass_autodetect)
{
#ifdef __cpp_lib_to_chars
	const T value =
	    parse<T>(str, skip_unsupported_prefix(str, 10), res, std::chars_format::general);
#else
	const T value = parse_float<T>(str, res);
#endif

	if (pass_autodetect) {
		*pass_autodetect = res;
	}

	return value;
}

template <>
inline float
string_to_number<float>(const char* str, const size_t, bool& res, bool* pass_autodetect)
{
	return string_to_float<float>(str, res, pass_autodetect);
}

template <>
inline double
string_to_number<double>(const char* str, const size_t, bool& res, bool* pass_autodetect)
{
	return string_to_float<double>(str, res, pass_autodetect);
}

/**
 * Integers. from_chars reports a value that does not fit in T as an error of its own,
 * so the range no longer has to be checked separately.
 */
template <typename T>
inline T string_to_integer(const char* str, const size_t base, bool& res)
{
	const char* const first = skip_unsupported_prefix(str, base);

	// from_chars would take the minus sign of a negative value as the start of an
	// unsigned one, and strtoul silently wrapped it around, so reject it up front
	if (std::is_unsigned_v<T> and *first == '-') {
		res = false;
		return 0;
	}

	return parse<T>(str, first, res, static_cast<int>(base));
}

#define PVCOP_STRING_TO_INTEGER(type)                                                              \
	template <>                                                                                    \
	inline type string_to_number<type>(const char* str, const size_t base, bool& res,               \
	                                   bool* /*pass_autodetect*/)                                   \
	{                                                                                              \
		return string_to_integer<type>(str, base, res);                                            \
	}

PVCOP_STRING_TO_INTEGER(int8_t)
PVCOP_STRING_TO_INTEGER(uint8_t)
PVCOP_STRING_TO_INTEGER(int16_t)
PVCOP_STRING_TO_INTEGER(uint16_t)
PVCOP_STRING_TO_INTEGER(int32_t)
PVCOP_STRING_TO_INTEGER(uint32_t)
PVCOP_STRING_TO_INTEGER(int64_t)
PVCOP_STRING_TO_INTEGER(uint64_t)

#undef PVCOP_STRING_TO_INTEGER

template <>
inline pvcop::db::uint128_t string_to_number<pvcop::db::uint128_t>(const char* /*str*/,
                                                                   const size_t /*base*/,
                                                                   bool& /*res*/,
                                                                   bool* /*pass_autodetect*/)
{
	assert(false && "not supported yet");
	return {};
}

} // namespace pvcop::types::__impl

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_IMPL_STRING_TO_NUMBER_H
