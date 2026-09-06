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

#ifndef PVCOP_TYPES_FORMATTER_H
#define PVCOP_TYPES_FORMATTER_H

#include <pvcop/types/formatter/formatter_interface.h>
#include <pvcop/core/impl/type_traits.h>
#include <pvcop/db/types.h>
#include <pvcop/sink.h>
#include <pvcop/core/impl/bit.h>

#include <type_traits>
#include <cstring>
#include <math.h>
#include <type_traits>

#include <pvlogger.h>

#define DEFINE_HASH_FUNCTION(type)                                                                 \
	namespace std                                                                                  \
	{                                                                                              \
	template <>                                                                                    \
	struct hash<type> {                                                                            \
		std::size_t operator()(const type& v) const                                                \
		{                                                                                          \
			using bitset_t = pvcop::core::bitset_cast_t<type>;                                     \
			return std::hash<bitset_t::value_type>()(bitset_t(v).cast());                          \
		}                                                                                          \
	};                                                                                             \
	}

namespace pvcop
{

namespace db
{

namespace __impl
{
template <typename T>
struct array_impl_string_converter;
}
}

namespace types
{

/**
 * This class defines a way to convert binary values to and from a
 * null-terminated string.
 */
template <typename T>
class formatter : public formatter_interface
{
	template <typename T2>
	friend struct db::__impl::array_impl_string_converter;

  public:
	using type = T;
	using value_type = typename core::__impl::type_traits<T>::value_type;
	using reference = typename core::__impl::type_traits<T>::accessor;

  public:
	/**
	 * Default constructor
	 *
	 * @param name the formatter name
	 */
	explicit formatter(const char* parameters) { set_parameters(parameters); }

  public:
	const char* parameters() const override { return _parameters.c_str(); }
	void set_parameters(const char* parameters) override
	{
		_parameters = parameters;
	}

  public:
	/**
	 * override pvcop::types::formatter_base::from_string
	 */
	bool from_string(const char* str,
	                 void* base_addr,
	                 size_t i,
	                 bool* pass_autodetect = nullptr) const final
	{
		auto&& value = core::__impl::type_traits<T>::get_accessor((value_type*)base_addr, i);

		int pa = 2; // simplify formatter API by using an undetermined initial autodetect state
		bool valid;
		if constexpr(std::is_same<T, bool>()) {
			valid = false;
			assert(false && "'from_string' method not implemended for this type");
		}
		else {
			valid = convert_from_string(str, value, (bool*)&pa);
		}

		if (pass_autodetect) {
			*pass_autodetect = valid or pa == 1;
		}

		return valid;
	}

	/**
	 * override pvcop::types::formatter_base::from_string
	 */
	bool from_string(const char* str, core::mempage& page, size_t row) const final
	{
		if constexpr(std::is_same<T, bool>()) {
			assert(false && "'from_string' method not implemended for 'bool' type");
			return false;
		}
		else {
			auto&& value = core::pagedarray<T>(page)[page.relative_index(row)];
			bool res = convert_from_string(str, value);

			if (not res) {
				// store invalid value
				size_t index = _invalid_write_dict->insert(str);
				if (index == db::write_dict::invalid_index or log2(index) > (sizeof(T) * 8)) {
					index = _invalid_write_dict->insert("?");
					pvlogger::warn() << "invalid value '" << str << "' failed to be stored correctly"
									<< std::endl;
				}
				value = core::cast_t<size_t, T>(index).cast();
			}

			return res;
		}
	}

	/**
	 * override pvcop::types::formatter_base::to_string
	 */
	int to_string(char* str, const size_t str_len, const void* base_addr, size_t i) const final
	{
		const T* data = static_cast<const T*>(base_addr);
		return convert_to_string(str, str_len, data[i]);
	}

	/**
	 * override pvcop::types::formatter_base::set_default_value
	 */
	void set_default_value(core::mempage& page, size_t row) const override final
	{
		if constexpr(std::is_same<T, bool>()) {
			assert(false && "'from_string' method not implemended for 'bool' type");
		}
		else {
			auto&& value = core::pagedarray<T>(page)[page.relative_index(row)];
			value = type();
		}
	}

  protected:
	/**
	 * Extract a value from a null-terminated string.
	 *
	 * @param str the buffer to read from
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return true on success; false otherwise
	 */
	virtual bool convert_from_string(const char* /*str*/,
	                                 reference /*value*/,
	                                 bool* /*pass_autodetect*/ = nullptr) const
	{
		assert(false && "'convert_from_string' method not implemended for this type");
		return false;
	}

	/**
	 * Format a value to a null-terminated string.
	 *
	 * @param str a buffer to write to
	 * @param str_len the buffer size
	 * @param value the value to format
	 *
	 * @return the written size or a negative value to report an error
	 */
	virtual int convert_to_string(char* /*str*/, const size_t /*str_len*/, const T& /*value*/) const
	{
		assert(false && "'convert_to_string' method not implemended for this type");
		return -1;
	};

  protected:
	std::string _parameters;
};

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_FORMATTER_H
