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

#ifndef __PVCOP_DB___IMPL_ARRAY_IMPL_INTERFACE_H__
#define __PVCOP_DB___IMPL_ARRAY_IMPL_INTERFACE_H__

#include <pvcop/core/array.h>
#include <pvcop/core/memarray.h>
#include <pvcop/db/selection.h>
#include <pvcop/db/exceptions/invalid_cast.h>
#include <pvcop/types/formatter/formatter_interface.h>
#include <pvcop/core/filearray.h>

#include <pvlogger.h>

#include <memory>
#include <cstring>
#include <typeinfo>

namespace pvcop
{

namespace db
{

template <template <typename> class CoreArrayBase, typename T>
class array_impl;

class read_dict;

/******************************************************************************
 *
 * pvcop::db::array_impl_interface
 *
 *****************************************************************************/

/**
 * Interface to the array implementation templated class.
 *
 * The main purpose of this class is to resolve virtual calls dispatch to the the array
 * implementation templated class in order to keep db::array class template free.
 */
class array_impl_interface
{
  public:
	array_impl_interface(const types::formatter_interface::sp& formatter,
	                     const read_dict* dict,
	                     const db::read_dict* invalid_dict,
	                     const core::selection& invalid_selection = {})
	    : _formatter(formatter)
	    , _read_dict(dict)
	    , _invalid_dict(invalid_dict)
	    , _invalid_selection(invalid_selection)
	{
	}

	virtual ~array_impl_interface() {}

  public:
	virtual bool operator==(const array_impl_interface& other) const = 0;

  public:
	/**
	 * Low-level array getters.
	 *
	 * @param T the internal value_type of the stored data
	 *
	 * @return the core::array low level representation
	 *         or an invalid array if the provided type T differs from the one in core::array
	 */
	template <typename T>
	inline const core::array<T>* to_core_array() const
	{
		pvlogger::trace() << "db::array_impl_interface::to_core_array()" << std::endl;

		const core::array<T>* arr = dynamic_cast<const core::array<T>*>(this);
		if (not arr) {
			throw exception::invalid_cast("bad cast for db::array of type '" + type() + "' into a core::array of another type");
		}
		return arr;
	}
	template <typename T>
	inline core::array<T>* to_core_array()
	{
		return const_cast<core::array<T>*>(
		    const_cast<const array_impl_interface*>(this)->to_core_array<T>());
	}

	/**
	 * pimpl resolvers.
	 *
	 * @param T the internal value_type of the stored data
	 * @param CoreArrayBase the internal container type of the array (core::array or
	 * core::memarray)
	 *
	 * @return the array_impl resolved template
	 *         or nullptr if the provided types "T"/"CoreArrayBase" differs from original ones
	 *
	 * @note this call has a runtime cost due to dynamic casting
	 */
	template <template <typename> class CoreArrayBase, typename T>
	inline const array_impl<CoreArrayBase, T>* to_db_array() const
	{
		pvlogger::trace() << "db::array_impl_interface::to_db_array()" << std::endl;

		return dynamic_cast<const array_impl<CoreArrayBase, T>*>(this);
	}
	template <template <typename> class CoreArrayBase, typename T>
	inline array_impl<CoreArrayBase, T>* to_db_array()
	{
		return const_cast<core::array<T>*>(
		    const_cast<const array_impl_interface*>(this)->to_db_array<CoreArrayBase, T>());
	}

  public:
	/**
	 * Retrieve the array type_id.
	 *
	 * @return the array type
	 */
	inline type_t type() const
	{
		assert(_formatter);

		pvlogger::trace() << "db::array_impl::type()" << std::endl;

		return _formatter->name();
	}

	int at(size_t i, char* str, const size_t str_len) const
	{
		assert(_formatter);

		if (not is_valid(i)) {
			return std::snprintf(str, str_len, "%s", invalid_value_at(i));
		}

		return _formatter->to_string(str, str_len, data(), i);
	}

	size_t max_string_length() const
	{
		if (not _read_dict) {
			return 0;
		}
		return strlen(*std::max_element(_read_dict->begin(), _read_dict->end(),
      		[](const auto& a, const auto& b) { return strlen(a) < strlen(b); }));
	}

	/**
	 * Specify if the array contains invalid values
	 *
	 * Returns the more detailed enum db::INVALID_TYPE that
	 * can also be used as a bool
	 */
	db::INVALID_TYPE has_invalid() const
	{
		if (not _invalid_selection) {
			return INVALID_TYPE::NONE;
		} else {
			if (_invalid_dict) {
				if (_invalid_dict->index("") != db::read_dict::invalid_index) {
					return static_cast<db::INVALID_TYPE>(INVALID_TYPE::EMPTY |
					                                     INVALID_TYPE::INVALID);
				} else {
					return INVALID_TYPE::INVALID;
				}
			} else {
				return INVALID_TYPE::EMPTY;
			}
		}
	}

	inline bool is_valid(size_t row) const
	{
		if (_invalid_selection) {
			return not _invalid_selection[row];
		}

		return true;
	}

	virtual const char* invalid_value_at(size_t row) const = 0;

	core::memarray<bool> valid_selection(const db::selection& sel = {}) const
	{
		if (_invalid_selection) {
			if (sel) {
				return sel & ~_invalid_selection;
			} else {
				return ~_invalid_selection;
			}
		} else {
			return sel;
		}
	}

	const core::selection& invalid_selection() const { return _invalid_selection; }
	core::selection& invalid_selection() { return _invalid_selection; }

	core::memarray<bool> invalid_selection(const db::selection& sel) const
	{
		if (_invalid_selection) {
			if (sel) {
				return sel & _invalid_selection;
			} else {
				return _invalid_selection;
			}
		} else {
			core::memarray<bool> inv_sel(size());
			std::fill(inv_sel.begin(), inv_sel.end(), false);
			return inv_sel;
		}
	};

	size_t valid_count(const db::selection& sel) const
	{
		const db::selection& vsel = valid_selection(sel);
		return vsel ? core::algo::bit_count(vsel) : 0;
	}

	size_t invalid_count(const db::selection& sel) const
	{
		const db::selection& isel = invalid_selection(sel);
		return isel ? core::algo::bit_count(isel) : 0;
	}

	const db::read_dict* invalid_dict() const { return _invalid_dict; }

	/**
	 * Returns a void pointer to the internal data.
	 */
	virtual void* data() = 0;
	virtual const void* data() const = 0;

	inline const types::formatter_interface::sp formatter() const { return _formatter; }
	inline void set_formatter(const types::formatter_interface::sp& formatter)
	{
		_formatter = formatter;
	}

	inline const read_dict* dict() const { return _read_dict; }

	/**
	 * Retrieve array size.
	 *
	 * @return the array size
	 */
	virtual size_t size() const = 0;

	virtual array_impl_interface* concat(const array_impl_interface* a2) const = 0;
	virtual array_impl_interface*
	concat(const std::vector<array_impl_interface*>& arrays) const = 0;

	virtual array_impl_interface* copy() const = 0;

	virtual array_impl_interface* slice(size_t pos, size_t len) const = 0;

	virtual array_impl_interface* to_array(std::vector<std::string>::const_iterator begin,
	                                       std::vector<std::string>::const_iterator end,
	                                       std::vector<std::string>* unconverable_values) const = 0;

	virtual array_impl_interface*
	to_array_if(std::vector<std::string>::const_iterator begin,
	            std::vector<std::string>::const_iterator end,
	            std::function<bool(const std::string& array_value, const std::string& exp_value)> f)
	    const = 0;

	/**
	 * Get selected values from indices/selection.
	 *
	 * Join a value array on an index/selection array.
	 *
	 * @note only works on indices/selection arrays.
	 *
	 * @see https://gitlab.srv.picviz/picviz/libpvcop/wikis/BAT#batleftfetchjoin
	 */
	virtual array_impl_interface* join(const array_impl_interface* a2) const = 0;
	virtual array_impl_interface* join(const db::selection& a2) const = 0;

	virtual void group(std::unique_ptr<array_impl_interface>& groups,
	                   std::unique_ptr<array_impl_interface>& extents,
	                   const db::selection& sel) const = 0;

	virtual array_impl_interface* sum(const db::selection& sel) const = 0;
	virtual array_impl_interface* average(const db::selection& sel) const = 0;
	virtual array_impl_interface* min(const db::selection& sel) const = 0;
	virtual array_impl_interface* max(const db::selection& sel) const = 0;
	virtual array_impl_interface* minmax(const db::selection& sel) const = 0;

	virtual array_impl_interface* divide(const array_impl_interface* divisors,
	                                     const db::selection& sel) const = 0;

	virtual array_impl_interface* group_count(const array_impl_interface* groups,
	                                          const array_impl_interface* extents) const = 0;
	virtual array_impl_interface* group_distinct_count(const array_impl_interface* groups,
	                                                   const array_impl_interface* extents,
	                                                   const db::selection& sel) const = 0;
	virtual array_impl_interface* group_sum(const array_impl_interface* groups,
	                                        const array_impl_interface* extents,
	                                        const db::selection& sel) const = 0;
	virtual array_impl_interface* group_min(const array_impl_interface* groups,
	                                        const array_impl_interface* extents,
	                                        const db::selection& sel) const = 0;
	virtual array_impl_interface* group_max(const array_impl_interface* groups,
	                                        const array_impl_interface* extents,
	                                        const db::selection& sel) const = 0;
	virtual array_impl_interface* group_average(const array_impl_interface* groups,
	                                            const array_impl_interface* extents,
	                                            const db::selection& sel) const = 0;

	virtual void range_select(const std::string& min,
	                          const std::string& max,
	                          const db::selection& intput_sel,
	                          db::selection& output_sel) const = 0;

	virtual void range_select(const std::string& min,
	                          const std::string& max,
	                          const db::range_t& intput_range,
	                          db::selection& output_sel) const = 0;

	virtual array_impl_interface* ratio_to_minmax(
	    double ratio1, double ratio2, const array_impl_interface* global_minmax) const = 0;

	virtual std::pair<double, double>
	minmax_to_ratio(const array_impl_interface* minmax,
	                const array_impl_interface* global_minmax /* = nullptr */) const = 0;

	virtual void histogram(size_t first,
	                       size_t last,
	                       const array_impl_interface* minmax,
	                       const array_impl_interface* sort_order,
	                       std::vector<size_t>& histogram) const = 0;

	virtual db::range_t
	equal_range(const array_impl_interface* minmax,
	            const array_impl_interface* sort_order /* = nullptr */) const = 0;

	virtual array_impl_interface* subtract(const array_impl_interface* value,
	                                       const array_impl_interface* groups) const = 0;

	virtual void subselect(const array_impl_interface* values,
	                       const db::selection& intput_sel,
	                       db::selection& output_sel) const = 0;

	virtual bool is_sorted() const = 0;

	virtual void parallel_sort(array_impl_interface* array) const = 0;
	virtual array_impl_interface* parallel_sort() const = 0;

  protected:
	types::formatter_interface::sp _formatter;
	const read_dict* _read_dict;
	const db::read_dict* _invalid_dict = nullptr; // is nullptr if the only value is ""
	core::selection _invalid_selection; // shadows array_impl::_invalid_selection on purpose to
	                                    // avoid dynamic dispatch cost
};

} // namespace db

} // namespace pvcop

#endif // __PVCOP_DB___IMPL_ARRAY_IMPL_INTERFACE_H__
