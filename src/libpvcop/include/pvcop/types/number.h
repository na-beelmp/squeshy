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

#ifndef PVCOP_TYPES_FORMATTER_NUMBER_H
#define PVCOP_TYPES_FORMATTER_NUMBER_H

#include <pvcop/types/formatter/formatter.h>

#include <pvcop/types/impl/string_to_number.h>
#include <pvcop/types/impl/number_to_string.h>

#include <pvcop/db/types.h>

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <unordered_map>

#include <cinttypes> // PRIxxx macro

namespace pvcop
{

namespace types
{

using numbers_t = pvcop::utils::type_tuple<bool,
                                           int8_t,
                                           int16_t,
                                           int32_t,
                                           int64_t,
                                           uint8_t,
                                           uint16_t,
                                           uint32_t,
                                           uint64_t,
                                           pvcop::db::uint128_t,
                                           float,
                                           double>;

template <typename T>
constexpr const char* type_string();
template <>
constexpr const char* type_string<bool>()
{
	return "bool";
}
template <>
constexpr const char* type_string<int8_t>()
{
	return "int8";
}
template <>
constexpr const char* type_string<int16_t>()
{
	return "int16";
}
template <>
constexpr const char* type_string<int32_t>()
{
	return "int32";
}
template <>
constexpr const char* type_string<int64_t>()
{
	return "int64";
}
template <>
constexpr const char* type_string<uint8_t>()
{
	return "uint8";
}
template <>
constexpr const char* type_string<uint16_t>()
{
	return "uint16";
}
template <>
constexpr const char* type_string<uint32_t>()
{
	return "uint32";
}
template <>
constexpr const char* type_string<uint64_t>()
{
	return "uint64";
}
template <>
constexpr const char* type_string<db::uint128_t>()
{
	return "uint128";
}
template <>
constexpr const char* type_string<float>()
{
	return "float";
}
template <>
constexpr const char* type_string<double>()
{
	return "double";
}

/**
 * This class defines a formatter for scalar values.
 */
template <typename T>
class formatter_number : public formatter<T>
{
  public:
	using reference = typename formatter<T>::reference;

  private:
	const std::unordered_map<std::string, std::string> format_map = {
	    {"bool", "%d"},         {"int8", "%" PRId8},    {"int16", "%" PRId16},
	    {"int32", "%" PRId32},  {"int64", "%" PRId64},  {"uint8", "%" PRIu8},
	    {"uint16", "%" PRIu16}, {"uint32", "%" PRIu32}, {"uint64", "%" PRIu64},
	    {"float", "%.9g"},      {"double", "%.17g"}};

  public:
	/**
	 * Default constructor
	 *
	 * @param format the format string
	 */
	formatter_number(const char* parameters) : formatter<T>(parameters)
	{
		const char* type_name = type_string<T>();
		_name = std::string("number_") + type_name;

		// Set a default parameters if none are specified
		if (not parameters or parameters[0] == '\0') {
			auto it = format_map.find(type_name);
			if (it != format_map.end()) {
				set_parameters(it->second.c_str());
				return;
			}
		}

		set_parameters(parameters);
	}

  public:
	std::string name() const override { return _name; }

	/*
	 * Don't show default parameters
	 */
	const char* parameters() const override
	{
		auto it = format_map.find(type_string<T>());
		if (it != format_map.end() && it->second == this->_parameters) {
			return "";
		}

		return this->_parameters.c_str();
	}

  protected:
	/**
	 * Overrides pvcop::types::formatter::from_string
	 *
	 * @param str a null-terminated string
	 * @param value the value to format
	 *
	 * @return true on success; false otherwise
	 */
	bool convert_from_string(const char* str, reference value, bool* pass_autodetect) const override
	{
		bool res = true;

		value = __impl::string_to_number<T>(str, _base, res, pass_autodetect);

		return res;
	}

	/**
	 * Overrides pvcop::types::formatter::to_string
	 *
	 * @param str a buffer to write to
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return the written size or a negative value to report an error
	 */
	int convert_to_string(char* str, const size_t str_len, const T& value) const override
	{
		return __impl::number_to_string<T>(str, str_len, this->_parameters.c_str(), _base, _float_precision,
		                                   _float_mode, value);
	}

  private:
	void set_parameters(const char* parameters) override
	{
		if (strcmp(parameters, "0x%x") == 0 or strcmp(parameters, "0x%lx") == 0 or strcmp(parameters, "0x%llx") == 0 or strcmp(parameters, "%#x") == 0) {
			_base = 16;
		} else if (strcmp(parameters, "%#o") == 0 or strcmp(parameters, "%#lo") == 0 or strcmp(parameters, "%#llo") == 0) {
			_base = 8;
		} else {
			_base = 10;
		}

		if (std::strchr(parameters, 'g') != nullptr) {
			_float_mode = 'g';
		} else if (std::strchr(parameters, 'e') != nullptr) {
			_float_mode = 'e';
		} else if (std::strchr(parameters, 'f') != nullptr) {
			_float_mode = 'f';
		}
		if (_float_mode != '\0') {
			std::string s = parameters;
			s.erase(std::remove_if(s.begin(), s.end(), [](char c) { return not std::isdigit(c); }),
			        s.end());
			if (not s.empty()) {
				_float_precision = std::strtol(s.c_str(), nullptr, 10);
			}
		}

		formatter<T>::set_parameters(parameters);
	}

  private:
	std::string _name;
	size_t _base;
	size_t _float_precision = 6;
	char _float_mode = '\0';
};

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_FORMATTER_NUMBER_H
