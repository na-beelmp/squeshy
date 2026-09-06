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

#ifndef __PVCOP_DB_ARRAY_IMPL_H__
#define __PVCOP_DB_ARRAY_IMPL_H__

#include <pvcop/core/impl/type_traits.h>
#include <pvcop/types/number.h>

#include <pvlogger.h>

#include <pvcop/core/filearray.h>
#include <pvcop/db/array.h>
#include <pvcop/db/read_dict.h>

#include <pvcop/core/selected_array.h>
#include <pvcop/core/ranged_array.h>
#include "db/array/primitive/index.h"
#include "db/array/primitive/sum.h"
#include "db/array/primitive/min_max.h"
#include "db/array/primitive/divide.h"
#include "db/array/primitive/group.h"
#include "db/array/primitive/group_distinct_count.h"
#include "db/array/primitive/group_count.h"
#include "db/array/primitive/group_sum.h"
#include "db/array/primitive/group_min.h"
#include "db/array/primitive/group_max.h"
#include "db/array/primitive/group_average.h"
#include "db/array/primitive/join.h"
#include "db/array/primitive/subselect.h"
#include "db/array/primitive/range_select.h"
#include "db/array/primitive/equal_range.h"
#include "db/array/primitive/subtract.h"
#include <pvcop/db/array_impl_interface.h>

//
#include <pvcop/types/duration.h>
//

#include <pvlogger.h>

#include <numeric>
#include <type_traits>

#include <tbb/parallel_sort.h>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace pvcop
{

namespace db
{

class read_dict;

namespace __impl
{

template <typename T, class = typename std::enable_if<std::is_standard_layout<T>::value and std::is_trivial<T>::value>::type>
types::formatter_interface::sp
get_proper_formatter(const types::formatter_interface::sp& /*formatter*/)
{
	return types::formatter_interface::sp(new types::formatter_number<T>(""));
}

template <typename T, class = typename std::enable_if<not (std::is_standard_layout<T>::value and std::is_trivial<T>::value)>::type>
const types::formatter_interface::sp&
get_proper_formatter(const types::formatter_interface::sp& formatter)
{
	return formatter;
}

template <typename T>
struct array_impl_string_converter {
	static T
	convert_from_string(const types::formatter<T>* formatter, const char* value_str, bool& res)
	{
		T value;
		res = formatter->convert_from_string(value_str, value);

		return value;
	}

	static int convert_to_string(const types::formatter<T>* formatter,
	                             char* str,
	                             const size_t str_len,
	                             const T& value)
	{
		return formatter->convert_to_string(str, str_len, value);
	}
};

template <>
struct array_impl_string_converter<bool> {
	static bool
	convert_from_string(const types::formatter<bool>* formatter, const char* value_str, bool& res)
	{
		//typename core::__impl::type_traits<bool>::value_type chunk;
		//typename core::__impl::type_traits<bool>::accessor bit(&chunk, 0);

		//res = formatter->convert_from_string(value_str, bit);

		//return bit;

		res = true;
		(void) formatter;
		(void) value_str;
		assert(false && "'convert_from_string' method not implemended for this type");
		return false;
	}

	static int convert_to_string(const types::formatter<bool>* formatter,
	                             char* str,
	                             const size_t str_len,
	                             bool value)
	{
		return formatter->convert_to_string(str, str_len, value);
	}
};

/**
 * Specialization for scalar types
 */
template <typename T>
struct parallel_sorter {
	static void parallel_sort(const array_impl_interface* array, array_impl_interface* ind)
	{
		core::array<index_t>* indexes = ind->to_core_array<index_t>();
		const core::array<T>* core_array = array->to_core_array<T>();

		assert(indexes);
		assert(array->size() >= indexes->size());

		const core::selection& inv_sel = array->invalid_selection();
		const db::read_dict* invalid_dict = array->invalid_dict();
		if (not inv_sel) {
			tbb::parallel_sort(indexes->begin(), indexes->end(), [&](index_t idx1, index_t idx2) {
				return (*core_array)[idx1] < (*core_array)[idx2];
			});
		} else if (not invalid_dict) {
			// invalid values are only empty values
			tbb::parallel_sort(indexes->begin(), indexes->end(), [&](index_t idx1, index_t idx2) {
				const bool vidx1 = not inv_sel[idx1];
				const bool vidx2 = not inv_sel[idx2];

				return (vidx1 && vidx2) ? (*core_array)[idx1] < (*core_array)[idx2] : vidx1;
			});
		} else {
			// index-sort the dictionary of invalid strings
			assert(invalid_dict);
			std::vector<index_t> invalid_dict_indexes(invalid_dict->size());
			std::iota(invalid_dict_indexes.begin(), invalid_dict_indexes.end(), 0);
			tbb::parallel_sort(invalid_dict_indexes.begin(), invalid_dict_indexes.end(),
			                   [&](index_t idx1, index_t idx2) {
				                   return strcmp(invalid_dict->key(idx1), invalid_dict->key(idx2)) <
				                          0;
			                   });

			// compute the "reverse" dictionary index
			std::vector<index_t> sorted(invalid_dict->size());
			std::iota(sorted.begin(), sorted.end(), 0);
			tbb::parallel_sort(sorted.begin(), sorted.end(), [&](index_t idx1, index_t idx2) {
				return invalid_dict_indexes[idx1] < invalid_dict_indexes[idx2];
			});

#if 0 // crashing : disabled for now
      // "As a rule of thumb, remove or optimize a branch if the branch mispredict ratio is
      // greater than about 8%
      // and the location of the branch is a hotspot in the execution profile relative to the
      // rest of the code.
      // from
      // https://software.intel.com/en-us/articles/avoiding-the-cost-of-branch-misprediction
			static constexpr double INVALID_RATIO = 0.1;
			if (core::algo::bit_count(inv_sel) > (size_t)(INVALID_RATIO * array->size())) {
				// branchless version
				tbb::parallel_sort(
				    indexes->begin(), indexes->end(), [&](index_t idx1, index_t idx2) {
					    const bool vidx1 = not inv_sel[idx1];
					    const bool vidx2 = not inv_sel[idx2];
					    const T v1 = (*core_array)[idx1];
					    const T v2 = (*core_array)[idx2];
					    index_t sv1 = sorted[core::cast_t<T, index_t>((*core_array)[idx1]).cast()];
					    index_t sv2 = sorted[core::cast_t<T, index_t>((*core_array)[idx2]).cast()];

					    return ((vidx1 & vidx2) & (v1 < v2)) |
					           ((not(vidx1 | vidx2)) & (sv1 < sv2)) | ((vidx1 ^ vidx2) & vidx1);
					});
			} else
#endif
			{
				tbb::parallel_sort(
				    indexes->begin(), indexes->end(), [&](index_t idx1, index_t idx2) {
					    const bool vidx1 = not inv_sel[idx1];
					    const bool vidx2 = not inv_sel[idx2];
					    if (vidx1 and vidx2) {
						    return (*core_array)[idx1] < (*core_array)[idx2];
					    } else if (not vidx1 and not vidx2) {
						    index_t sv1 =
						        sorted[core::cast_t<T, index_t>((*core_array)[idx1]).cast()];
						    index_t sv2 =
						        sorted[core::cast_t<T, index_t>((*core_array)[idx2]).cast()];
						    return sv1 < sv2;

					    } else {
						    return vidx1;
					    }
				    });
			}
		}

		if (inv_sel) {
			// sort invalid selection as well
			core::selection& inv_sel = ind->invalid_selection();
			const size_t bc = pvcop::core::algo::bit_count(inv_sel);
			std::fill(inv_sel.begin(), inv_sel.begin() + (inv_sel.size() - bc + 1), false);
			std::fill(inv_sel.begin() + (inv_sel.size() - bc), inv_sel.end(), true);
		}
	}
};

/**
 * Specialization for string type
 */
template <>
struct parallel_sorter<string_index_t> {
	static void parallel_sort(const array_impl_interface* array, array_impl_interface* ind)
	{
		const read_dict* dict = array->dict();

		assert(dict);
		assert(array->size() >= ind->size());

		core::array<index_t>* indexes = ind->to_core_array<index_t>();
		const core::array<string_index_t>* core_array = array->to_core_array<string_index_t>();

		/**
		 * Default sort is best suited in case entropy is too high
		 */
		static constexpr double ENTROPY_RATIO = 0.05;
		if (((double)dict->size() / array->size()) < ENTROPY_RATIO) {
			std::vector<index_t> dict_indexes(dict->size());
			std::vector<index_t> dict_indexes_reverse(dict_indexes.size());
			std::iota(dict_indexes.begin(), dict_indexes.end(), 0);

			// index-sort the dictionary of strings
			tbb::parallel_sort(dict_indexes.begin(), dict_indexes.end(),
			                   [&](index_t idx1, index_t idx2) {
				                   return strcmp(dict->key(idx1), dict->key(idx2)) < 0;
			                   });

			// compute the "reverse" dictionary index
			std::iota(dict_indexes_reverse.begin(), dict_indexes_reverse.end(), 0);
			tbb::parallel_sort(dict_indexes_reverse.begin(), dict_indexes_reverse.end(),
			                   [&](index_t idx1, index_t idx2) {
				                   return dict_indexes[idx1] < dict_indexes[idx2];
			                   });

			// sort the whole array using the "reverse" indirection
			tbb::parallel_sort(indexes->begin(), indexes->end(), [&](index_t idx1, index_t idx2) {
				return dict_indexes_reverse[(*core_array)[idx1]] <
				       dict_indexes_reverse[(*core_array)[idx2]];
			});
		} else {
			tbb::parallel_sort(indexes->begin(), indexes->end(), [&](index_t idx1, index_t idx2) {
				return strcmp(dict->key((*core_array)[idx1]), dict->key((*core_array)[idx2])) < 0;
			});
		}
	}
};

} // namespace __impl

/******************************************************************************
 *
 * pvcop::db::array_impl
 *
 *****************************************************************************/

/**
 * This class holds the generic implementations of the database primitive
 *methods.
 *
 * It implements the db::array_impl_interface class and inherit as well from a
 *"CoreArrayBase" class
 * (core::array, which doesn't hold memory ownership, or core::memarray which
 *does)
 * that can be directly used in order to provide an efficient way to access the
 *array underlying
 *data.
 */
template <template <typename> class CoreArrayBase, typename T>
class array_impl : public array_impl_interface, public CoreArrayBase<T>
{
  public:
	using value_type = T;

  public:
	/**
	 * Initialization constructor.
	 *
	 * @param type the type of the array as a string
	 * @param count the number of elements contained in the array
	 *
	 * @warning this constructor must not be used to create instances of string
	 *typed
	 * pvcop::db::array
	 */
	array_impl(size_t count,
	           const types::formatter_interface::sp& formatter,
	           const read_dict* dict,
	           const read_dict* invalid_dict,
	           core::memarray<bool> invalid_selection,
	           bool init = false)
	    : array_impl_interface(formatter, dict, invalid_dict)
	    , CoreArrayBase<T>(count, init)
	    , _invalid_selection(std::move(invalid_selection))
	{
		array_impl_interface::_invalid_selection = this->_invalid_selection;

		pvlogger::trace() << "db::array_impl::array_impl(" << type() << ", " << count << ")"
		                  << std::endl;
	}

	/**
	 * Slice constructor.
	 *
	 * @param pimpl the original array pimpl
	 * @param pos the start position in @a array
	 * @param len the size of the new array
	 */
	array_impl(const array_impl_interface* pimpl, size_t pos, size_t len)
	    : array_impl_interface(pimpl->formatter(), pimpl->dict(), pimpl->invalid_dict())
	    , CoreArrayBase<T>(*pimpl->to_core_array<T>(), pos, len)
	    , _invalid_selection(pimpl->invalid_selection()
	                             ? core::selection(pimpl->invalid_selection(), pos, len)
	                             : core::selection())
	{
		array_impl_interface::_invalid_selection = this->_invalid_selection;

		assert(type() == pimpl->type());
		pvlogger::trace() << "db::array_impl::array_impl(" << pimpl << ", " << pos << ", " << len
		                  << ")" << std::endl;
	}

	/**
	 * Constructor from memarray
	 *
	 * @param mem_array the memarray array
	 * @param type the associated type of the array
	 * @param dict the associated dict
	 * @param formatter the associated formatter
	 */
	array_impl(CoreArrayBase<T>&& memarray,
	           const types::formatter_interface::sp& formatter,
	           const read_dict* dict = nullptr,
	           const read_dict* invalid_dict = nullptr,
	           core::memarray<bool> invalid_selection = {})
	    : array_impl_interface(formatter, dict, invalid_dict, invalid_selection)
	    , CoreArrayBase<T>(std::forward<CoreArrayBase<T>>(memarray))
	    , _invalid_selection(std::move(invalid_selection))
	{
		pvlogger::trace() << "db::array_impl::array_impl(memarray(" << &memarray << "))"
		                  << std::endl;
	}

	/**
	 * Mapped file array constructor
	 *
	 * @param type the type of the array
	 * @param data the base address of the file mapped array
	 * @param count the number of elements contained in the array
	 * @param rf the release function to use when the array will be freed
	 * @param formatter the proper formatter to convert data to string (optional)
	 */
	array_impl(void* data,
	           size_t count,
	           const types::formatter_interface::sp& formatter,
	           const core::filearray_release_f& rf,
	           const read_dict* dict,
	           core::filearray<bool> invalid_selection,
	           const read_dict* invalid_values_dict)
	    : array_impl_interface(formatter, dict, invalid_values_dict, invalid_selection)
	    , CoreArrayBase<T>(
	          static_cast<typename core::__impl::type_traits<T>::value_type*>(data), count, rf)
	    , _invalid_selection(std::move(invalid_selection))
	{
		pvlogger::trace() << "db::array_impl::array_impl(" << data << ")" << std::endl;
	}

	/**
	 * Destructor.
	 */
	~array_impl() override { pvlogger::trace() << "db::array_impl::~array_impl()" << std::endl; }

  public:
	/**
	 * Low-level array getters.
	 *
	 * @param T the internal value_type of the stored data.
	 *
	 * @return the core::array low-level representation
	 *         or an invalid array if the provided type T differs from the one in
	 *core::array.
	 */
	inline const core::array<T>* to_core_array() const
	{
		pvlogger::trace() << "db::array_impl::to_core_array()" << std::endl;

		return static_cast<const core::array<T>*>(this);
	}
	inline core::array<T>* to_core_array()
	{
		return const_cast<core::array<T>*>(
		    static_cast<const array_impl<core::array, T>*>(this)->to_core_array());
	}

  public:
	bool operator==(const array_impl_interface& other) const override
	{
		if (type() == other.type()) {
			const auto& other_native = *other.to_core_array<T>();
			const auto& array_native = *to_core_array();

			return array_native == other_native;
		}

		return false;
	}

  public:
	size_t size() const override { return core::array<T>::size(); }

	const void* data() const override { return core::array<T>::data(); }

	void* data() override
	{
		return const_cast<void*>(static_cast<const array_impl<CoreArrayBase, T>*>(this)->data());
	}

	const char* invalid_value_at(size_t row) const override
	{
		if (not _invalid_dict) {
			return "";
		} else {
			db::read_dict::index_type dict_index =
			    core::cast_t<T, db::read_dict::index_type>(((T*)data())[row]).cast();
			return _invalid_dict->key(dict_index);
		}
	}

  public:
	array_impl_interface* slice(size_t pos, size_t len) const override
	{
		pvlogger::trace() << "db::array_impl::slice(" << pos << ", " << len << ")" << std::endl;

		array_impl_interface* slice = new array_impl<core::array, T>(this, pos, len);

		return slice;
	}

	/**
	 * Concatenate two array and return a new one.
	 */
	array_impl_interface* concat(const array_impl_interface* rhs) const override
	{
		pvlogger::trace() << "db::array_impl::concat(const array_impl_interface*)" << std::endl;

		assert(rhs);

		core::memarray<bool> invalid_sel;
		if (invalid_selection() or rhs->invalid_selection()) {
			invalid_sel = core::memarray<bool>(size() + rhs->size(), true);
			if (invalid_selection()) {
				const core::selection& sel = invalid_selection();
				for (size_t i = 0; i < size(); i++) {
					invalid_sel[i] = sel[i];
				}
			}
			if (rhs->invalid_selection()) {
				const core::selection& sel = rhs->invalid_selection();
				const size_t start = size();
				for (size_t i = 0; i < sel.size(); i++) {
					invalid_sel[i + start] = sel[i];
				}
			}
		}

		array_impl_interface* concat_array = new array_impl<core::memarray, T>(
		    size() + rhs->size(), formatter(), dict(), invalid_dict(), std::move(invalid_sel));

		auto& concat_array_native = *concat_array->to_core_array<T>();
		const auto& array1_native = *to_core_array();
		const auto& array2_native = *rhs->to_core_array<T>();

		concat_array_native.copy_from(array1_native, 0, array1_native.size());
		concat_array_native.copy_from(array2_native, array1_native.size(), array2_native.size());

		return concat_array;
	}

	array_impl_interface* concat(const std::vector<array_impl_interface*>& as) const override
	{
		pvlogger::trace() << "db::array_impl::concat(const std::vector<array_impl_interface*>)"
		                  << std::endl;

		size_t size = std::accumulate(as.begin(), as.end(), (size_t)0,
		                              [](size_t s, const auto* a) { return s + a->size(); });
		bool has_invalid =
		    std::any_of(as.begin(), as.end(), [](const auto* a) { return a->has_invalid(); });

		core::memarray<bool> invalid_sel;
		if (has_invalid) {
			invalid_sel = core::memarray<bool>(size);
			size_t i = 0;
			for (size_t n = 0; n < as.size(); n++) {
				const core::selection& inv_sel = as[n]->invalid_selection();
				for (size_t j = 0; j < as[n]->size(); j++, i++) {
					invalid_sel[i] = inv_sel ? inv_sel[j] : false;
				}
			}
		}

		array_impl_interface* concat_array = new array_impl<core::memarray, T>(
		    size, formatter(), dict(), invalid_dict(), std::move(invalid_sel));

		auto& concat_array_native = *concat_array->to_core_array<T>();

		size_t pos = 0;
		for (size_t n = 0; n < as.size(); n++) {
			const array_impl_interface* array = as[n];
			const auto& array_native = *array->to_core_array<T>();

			concat_array_native.copy_from(array_native, pos, array_native.size());
			pos += array->size();
		}

		return concat_array;
	}

	/**
	 * See db::array::copy documentation
	 */
	array_impl_interface* copy() const override
	{
		pvlogger::trace() << "db::array_impl::copy()" << std::endl;

		array_impl_interface* copied_array = new array_impl<core::memarray, T>(
		    size(), formatter(), dict(), invalid_dict(), invalid_selection());

		auto& copied_array_native = *copied_array->to_core_array<T>();
		const auto& this_native = *to_core_array();
		copied_array_native.copy_from(this_native, 0, size());

		return copied_array;
	}

	/**
	 * See db::array_base::to_array documentation
	 */
	array_impl_interface* to_array(std::vector<std::string>::const_iterator begin,
	                               std::vector<std::string>::const_iterator end,
	                               std::vector<std::string>* unconvertable_values) const override
	{
		types::formatter<T>* typed_formatter = static_cast<types::formatter<T>*>(_formatter.get());

		const size_t max_count = std::distance(begin, end);

		size_t converted_count = 0;
		core::memarray<T> converted_array(max_count);

		core::memarray<bool> invalid_sel;
		if (_invalid_selection) {
			invalid_sel = core::memarray<bool>(max_count);
			std::fill(invalid_sel.begin(), invalid_sel.end(), false);
		}

		for (size_t i = 0; i < max_count; i++) {
			bool converted = false;
			const std::string& value_str = (*(begin + i));

			// (1) try to convert to valid value
			T v = __impl::array_impl_string_converter<T>::convert_from_string(
			    typed_formatter, value_str.c_str(), converted);
			if (converted) {
				converted_array[converted_count] = v;
				converted_count++;
			}

			// (2) if failure, try to convert to invalid value
			if (_invalid_selection and not converted) {
				db::read_dict::index_type index =
				    _invalid_dict ? _invalid_dict->index(value_str.c_str()) : 0;
				if (index != db::read_dict::invalid_index) {
					converted = true;
					if constexpr (std::is_trivial_v<T>) {
						converted_array[converted_count] = index;
					}
					else {
						converted_array[converted_count] = core::cast_t<db::read_dict::index_type, T>(index).cast();
					}
					invalid_sel[converted_count] = true;
					converted_count++;
				}
			}

			// (3) if all kind of conversions failed, add to the list of unconvertable
			// values
			if (unconvertable_values and not converted) {
				unconvertable_values->emplace_back(value_str);
			}
		}

		if (converted_count < max_count) {
			converted_array.resize(converted_count);
			if (invalid_sel) {
				invalid_sel.resize(converted_count);
			}
		}

		return new array_impl<core::memarray, T>(std::move(converted_array), formatter(), dict(),
		                                         invalid_dict(), std::move(invalid_sel));
	}

	/**
	 * See db::array_base::to_array documentation
	 */
	array_impl_interface*
	to_array_if(std::vector<std::string>::const_iterator begin,
	            std::vector<std::string>::const_iterator end,
	            std::function<bool(const std::string& array_value, const std::string& exp_value)>
	                custom_match_f) const override
	{
		const core::array<T>& in_core_array = *to_core_array();
		core::memarray<T> converted_array(size());

		types::formatter<T>* typed_formatter = static_cast<types::formatter<T>*>(_formatter.get());

		core::memarray<bool> invalid_sel;
		if (_invalid_selection) {
			invalid_sel = core::memarray<bool>(in_core_array.size());
			std::fill(invalid_sel.begin(), invalid_sel.end(), false);
		}

		constexpr size_t MAX_LENGTH = types::formatter_interface::MAX_STRING_LENGTH;
		char buffer[MAX_LENGTH];
		const char* in_array_str;
		size_t converted_count = 0;
		for (size_t row = 0; row < in_core_array.size(); row++) {
			const T& value = in_core_array[row];

			bool invalid = _invalid_selection and _invalid_selection[row];
			if (invalid) {
				in_array_str =
				    _invalid_dict
				        ? _invalid_dict->key(
				              core::cast_t<T, db::read_dict::index_type>(((T*)data())[row]).cast())
				        : "";
				assert(in_array_str);
			} else {
				__impl::array_impl_string_converter<T>::convert_to_string(typed_formatter, buffer,
				                                                          MAX_LENGTH, value);
				in_array_str = buffer;
			}

			// find if an expression match the value
			std::vector<std::string>::const_iterator it =
			    std::find_if(begin, end, [&](std::string const& exp) {
				    return custom_match_f(in_array_str, exp);
			    });
			if (it != end) {
				converted_array[converted_count] = value;
				if (invalid_sel) {
					invalid_sel[converted_count] = invalid;
				}
				converted_count++;
			}
		}

		if (converted_count < size()) {
			converted_array.resize(converted_count);
			if (invalid_sel) {
				invalid_sel.resize(converted_count);
			}
		}

		return new array_impl<core::memarray, T>(std::move(converted_array), formatter(), dict(),
		                                         invalid_dict(), std::move(invalid_sel));
	}

	/**
	 * Join a value array with an index array.
	 *
	 * @return New array without joins informations.
	 */
	array_impl_interface* join(const array_impl_interface* array_interface) const override
	{
		pvlogger::trace() << "db::array_impl::join(const array_impl_interface*)" << std::endl;

		const auto& indices = *array_interface->to_core_array<index_t>();
		auto& data_array = *to_core_array();

		return new array_impl<core::memarray, T>(primitive::join(data_array, indices), formatter(),
		                                         dict(), invalid_dict(),
		                                         array_interface->invalid_selection());
	}

	/**
	 * Join a value array with a selection array.
	 *
	 * @return New array without joins informations.
	 */
	array_impl_interface* join(const db::selection& sel) const override
	{
		pvlogger::trace() << "db::array_impl::join(const array_impl_interface*)" << std::endl;

		auto& data_array = *to_core_array();

		core::memarray<T> joined_array;
		core::memarray<bool> invalid_sel;
		std::tie(joined_array, invalid_sel) = primitive::join(data_array, sel, invalid_selection());
		return new array_impl<core::memarray, T>(std::move(joined_array), formatter(), dict(),
		                                         invalid_dict(), std::move(invalid_sel));
	}

	array_impl_interface* sum(const db::selection& sel) const override
	{
		const core::array<T>& array = *to_core_array();

		using promoted_type = typename pvcop::db::primitive::sum_type<T>::type;

		return new array_impl<core::memarray, promoted_type>(
		    primitive::sum(core::make_selected_array(array, valid_selection(sel))),
		    __impl::get_proper_formatter<promoted_type>(formatter()), dict(), invalid_dict());
	}

	array_impl_interface* min(const db::selection& sel) const override
	{
		const core::array<T>& array = *to_core_array();

		return new array_impl<core::memarray, T>(
		    primitive::min(core::make_selected_array(array, valid_selection(sel))), formatter(),
		    dict(), invalid_dict());
	}

	array_impl_interface* max(const db::selection& sel) const override
	{
		const core::array<T>& array = *to_core_array();

		return new array_impl<core::memarray, T>(
		    primitive::max(core::make_selected_array(array, valid_selection(sel))), formatter(),
		    dict(), invalid_dict());
	}

	array_impl_interface* minmax(const db::selection& sel) const override
	{
		const core::array<T>& array = *to_core_array();

		return new array_impl<core::memarray, T>(
		    primitive::minmax(core::make_selected_array(array, valid_selection(sel))), formatter(),
		    dict(), invalid_dict());
	}

	array_impl_interface* divide(const array_impl_interface* divisors,
	                             const db::selection& sel) const override
	{
		const core::array<T>& array = *to_core_array();
		const core::array<uint64_t>& div = *divisors->to_core_array<uint64_t>();

		auto divided_array = primitive::divide(array, div, valid_selection(sel));

		using array_type = typename decltype(divided_array)::value_type;
		return new array_impl<core::memarray, array_type>(
		    std::move(divided_array), __impl::get_proper_formatter<array_type>(formatter()));
	}

	array_impl_interface* average(const db::selection& sel) const override
	{
		size_t vcount = valid_count(sel);
		if (vcount == 0) {
			return nullptr;
		}

		std::unique_ptr<array_impl_interface> sum_array(sum(sel));

		core::memarray<uint64_t> div(1);
		div[0] = vcount;
		const auto& divisor =
		    array_impl<core::memarray, decltype(div)::value_type>(std::move(div), {});

		return sum_array->divide(&divisor, {});
	}

	/**
	 * Group elements from the array.
	 *
	 * @param[out] groups: contains mapping from elements to group.
	 * @param[out] extents: contains mapping from group to first elements
	 *
	 * @note https://gitlab.srv.picviz/picviz/libpvcop/wikis/BAT#batgroup
	 */
	void group(std::unique_ptr<array_impl_interface>& groups,
	           std::unique_ptr<array_impl_interface>& extents,
	           const db::selection& sel) const override
	{

		// Outputs have to be empty
		assert(groups.get() == nullptr);
		assert(extents.get() == nullptr);

		// get value array
		const core::array<T>& array = *to_core_array();

		core::memarray<index_t> groups_array;
		core::memarray<bool> invalid_groups_sel;
		core::memarray<index_t> extents_array;
		core::memarray<bool> invalid_extents_sel;
		std::tie(groups_array, invalid_groups_sel, extents_array, invalid_extents_sel) =
		    primitive::group(core::make_selected_array(array, sel), invalid_selection(sel));

		groups.reset(new array_impl<core::memarray, index_t>(
		    std::move(groups_array),
		    types::formatter_interface::sp(new types::formatter_number<index_t>("")), nullptr,
		    invalid_dict(), std::move(invalid_groups_sel)));
		extents.reset(new array_impl<core::memarray, index_t>(
		    std::move(extents_array),
		    types::formatter_interface::sp(new types::formatter_number<index_t>("")), nullptr,
		    invalid_dict(), std::move(invalid_extents_sel)));
	}

	array_impl_interface* group_distinct_count(const array_impl_interface* groups,
	                                           const array_impl_interface* extents,
	                                           const db::selection& sel) const override
	{
		pvlogger::trace() << "db::array_impl::group_distinct_count(...)" << std::endl;

		const core::array<T>& array = *to_core_array();
		const core::array<index_t>& groups_array = *groups->to_core_array<index_t>();
		const core::array<index_t>& extents_array = *extents->to_core_array<index_t>();

		auto distinct_count_array = primitive::group_distinct_count(
		    core::make_selected_array(array, sel), groups_array, extents_array);

		return new array_impl<core::memarray, typename decltype(distinct_count_array)::value_type>(
		    std::move(distinct_count_array),
		    types::formatter_interface::sp(new types::formatter_number<index_t>("")));
	}

	/**
	 * Sum values in the same group.
	 *
	 * @note It doesn't really use group_count first as output type may differ.
	 */
	array_impl_interface* group_sum(const array_impl_interface* groups,
	                                const array_impl_interface* extents,
	                                const db::selection& sel) const override
	{
		pvlogger::trace() << "db::array_impl::group_sum(...)" << std::endl;

		const core::array<T>& array = *to_core_array();
		const core::array<index_t>& groups_array = *groups->to_core_array<index_t>();

		auto sum_array = primitive::group_sum(core::make_selected_array(array, sel), groups_array,
		                                      extents->size(), invalid_selection());

		using array_type = typename decltype(sum_array)::value_type;
		return new array_impl<core::memarray, array_type>(
		    std::move(sum_array), __impl::get_proper_formatter<array_type>(formatter()));
	}

	/**
	 * Count number of value for each group.
	 *
	 * @note It doesn't depend on the current class, it could be a static
	 *function.
	 *
	 */
	array_impl_interface* group_count(const array_impl_interface* groups,
	                                  const array_impl_interface* extents) const override
	{
		pvlogger::trace() << "db::array_impl::group_count(...)" << std::endl;

		const core::array<index_t>& groups_array = *groups->to_core_array<index_t>();

		return new array_impl<core::memarray, index_t>(
		    primitive::group_count(groups_array, extents->size()),
		    types::formatter_interface::sp(new types::formatter_number<index_t>("")));
	}

	/**
	 * Get minimum values for each group.
	 */
	array_impl_interface* group_min(const array_impl_interface* groups,
	                                const array_impl_interface* extents,
	                                const db::selection& sel) const override
	{
		pvlogger::trace() << "db::array_impl::group_min(...)" << std::endl;

		const core::array<T>& array = *to_core_array();
		const core::array<index_t>& groups_array = *groups->to_core_array<index_t>();
		const core::array<index_t>& extents_array = *extents->to_core_array<index_t>();

		core::memarray<T> min_array;
		core::memarray<bool> invalid_sel;
		std::tie(min_array, invalid_sel) =
		    primitive::group_min(core::make_selected_array(array, sel), groups_array,
		                         extents_array.size(), invalid_selection());

		return new array_impl<core::memarray, T>(std::move(min_array), formatter(), dict(), nullptr,
		                                         std::move(invalid_sel));
	}

	/**
	 * Get  maximum values for each group.
	 */
	array_impl_interface* group_max(const array_impl_interface* groups,
	                                const array_impl_interface* extents,
	                                const db::selection& sel) const override
	{
		pvlogger::trace() << "db::array_impl::group_max(...)" << std::endl;

		const core::array<T>& array = *to_core_array();
		const core::array<index_t>& groups_array = *groups->to_core_array<index_t>();
		const core::array<index_t>& extents_array = *extents->to_core_array<index_t>();

		core::memarray<T> max_array;
		core::memarray<bool> invalid_sel;
		std::tie(max_array, invalid_sel) =
		    primitive::group_max(core::make_selected_array(array, sel), groups_array,
		                         extents_array.size(), invalid_selection());

		return new array_impl<core::memarray, T>(std::move(max_array), formatter(), dict(), nullptr,
		                                         std::move(invalid_sel));
	}

	/**
	 * Get average values for each group.
	 */
	array_impl_interface* group_average(const array_impl_interface* groups,
	                                    const array_impl_interface* extents,
	                                    const db::selection& sel) const override
	{
		pvlogger::trace() << "db::array_impl::group_average(...)" << std::endl;

		const core::array<T>& array = *to_core_array();
		const core::array<index_t>& groups_array = *groups->to_core_array<index_t>();
		const core::array<index_t>& extents_array = *extents->to_core_array<index_t>();

		auto avg = primitive::group_average(core::make_selected_array(array, sel), groups_array,
		                                    extents_array.size(), invalid_selection());

		using array_type = typename decltype(avg)::value_type;
		return new array_impl<core::memarray, array_type>(
		    std::move(avg), __impl::get_proper_formatter<array_type>(formatter()));
	}

	void range_select(const std::string& min_str,
	                  const std::string& max_str,
	                  const db::selection& intput_sel,
	                  db::selection& output_sel) const override
	{
		assert(std::is_arithmetic<T>::value && "range_select can apply only on arithmetic array");

		types::formatter<T>* typed_formatter = static_cast<types::formatter<T>*>(_formatter.get());

		bool min_ok, max_ok;
		T min = __impl::array_impl_string_converter<T>::convert_from_string(
		    typed_formatter, min_str.c_str(), min_ok);
		T max = __impl::array_impl_string_converter<T>::convert_from_string(
		    typed_formatter, max_str.c_str(), max_ok);

		if (not min_ok || not max_ok) {
			throw db::exception::unable_to_convert_input_error();
		}

		const core::array<T>& array = *to_core_array();

		primitive::range_select(core::make_selected_array(array, valid_selection(intput_sel)), min,
		                        max, output_sel);
	}

	void range_select(const std::string& min_str,
	                  const std::string& max_str,
	                  const db::range_t& intput_range,
	                  db::selection& output_sel) const override
	{
		assert(std::is_arithmetic<T>::value && "range_select can apply only on arithmetic array");

		types::formatter<T>* typed_formatter = static_cast<types::formatter<T>*>(_formatter.get());

		bool min_ok, max_ok;
		T min = __impl::array_impl_string_converter<T>::convert_from_string(
		    typed_formatter, min_str.c_str(), min_ok);
		T max = __impl::array_impl_string_converter<T>::convert_from_string(
		    typed_formatter, max_str.c_str(), max_ok);

		if (not min_ok || not max_ok) {
			throw db::exception::unable_to_convert_input_error();
		}

		const core::array<T>& array = *to_core_array();

		primitive::range_select(core::make_ranged_array(array, intput_range), min, max, output_sel);
	}

	db::range_t equal_range(const array_impl_interface* minmax,
	                        const array_impl_interface* sort_order /* = nullptr */) const override
	{
		const core::array<T>& array = *to_core_array();
		const core::array<T>& core_minmax = *minmax->to_core_array<T>();
		const core::array<index_t>* core_sort_order = nullptr;
		if (sort_order) {
			core_sort_order = sort_order->to_core_array<index_t>();
		}

		return primitive::equal_range(array, _invalid_selection, core_minmax, core_sort_order);
	}

	array_impl_interface*
	ratio_to_minmax(double ratio1,
	                double ratio2,
	                const array_impl_interface* global_minmax /* = nullptr */) const override
	{
		array_impl_interface* local_minmax =
		    new array_impl<core::memarray, T>((size_t)2, formatter(), dict(), invalid_dict(), {});

		if (global_minmax) {
			_global_minmax.reset(global_minmax->copy());
		}
		if (not _global_minmax) {
			_global_minmax.reset(minmax({}));
		}

		pvcop::core::array<T> core_rel_minmax = *local_minmax->to_core_array<T>();
		const pvcop::core::array<T>& core_minmax =
		    *_global_minmax.get()->template to_core_array<T>();

		T abs_min = core_minmax[0];
		T abs_max = core_minmax[1];

		auto duration = abs_max - abs_min;

		if constexpr (std::is_same<T, boost::posix_time::ptime>::value or
		              std::is_same<T, boost::posix_time::time_duration>::value) {
			auto duration_us = duration.total_microseconds();
			// int64_t, not long: long is 32 bits on Windows, and a range of more than
			// about half an hour is already more microseconds than that holds. The cast
			// truncated, the upper bound landed below the lower one, and every range
			// query against it came back empty.
			core_rel_minmax[0] =
			    abs_min + boost::posix_time::microseconds((int64_t)(ratio1 * duration_us));
			core_rel_minmax[1] =
			    abs_min + boost::posix_time::microseconds((int64_t)(ratio2 * duration_us));
		} else {
			core_rel_minmax[0] = abs_min + (duration * ratio1);
			core_rel_minmax[1] = abs_min + (duration * ratio2);
		}

		return local_minmax;
	}

	std::pair<double, double>
	minmax_to_ratio(const array_impl_interface* local_minmax,
	                const array_impl_interface* global_minmax /* = nullptr */) const override
	{
		if (global_minmax) {
			_global_minmax.reset(global_minmax->copy());
		}
		if (not _global_minmax) {
			_global_minmax.reset(minmax({}));
		}

		const pvcop::core::array<T>& core_global_minmax = *global_minmax->to_core_array<T>();
		const pvcop::core::array<T>& core_local_minmax = *local_minmax->to_core_array<T>();

		T global_min = core_global_minmax[0];
		T global_max = core_global_minmax[1];
		T local_min = core_local_minmax[0];
		T local_max = core_local_minmax[1];

		auto duration = global_max - global_min;
		double ratio1;
		double ratio2;

		if constexpr (std::is_same<T, boost::posix_time::ptime>::value or
		              std::is_same<T, boost::posix_time::time_duration>::value) {
			auto duration_us = duration.total_microseconds();
			ratio1 = (local_min - global_min).total_microseconds() / (double)duration_us;
			ratio2 = (local_max - global_min).total_microseconds() / (double)duration_us;
		} else if constexpr (std::is_same<T, boost::posix_time::time_duration>::value) {
			throw db::exception::unable_to_convert_input_error();
		} else {
			ratio1 = (local_min - global_min) / (double)duration;
			ratio2 = (local_max - global_min) / (double)duration;
		}

		(void)duration;
		(void)global_min;
		(void)global_max;
		(void)local_min;
		(void)local_max;

		return std::make_pair(ratio1, ratio2);
	}

	void histogram(size_t first,
	               size_t last,
	               const array_impl_interface* minmax,
	               const array_impl_interface* sort_order,
	               std::vector<size_t>& histogram) const override
	{
		const core::array<T>& core_array = *to_core_array();
		const core::array<T>& core_minmax = *minmax->to_core_array<T>();
		const core::array<index_t>& sort =
		    sort_order ? *sort_order->to_core_array<index_t>() : core::array<index_t>();

		T min = core_minmax[0];
		T max = core_minmax[1];

		const pvcop::db::selection& invalid_sel = invalid_selection();

		using interval_t =
		    typename std::conditional<std::is_same<T, boost::posix_time::ptime>::value or
		                                  std::is_same<T, boost::posix_time::time_duration>::value,
		                              boost::posix_time::time_duration, double>::type;
		const interval_t& interval = (interval_t)(max - min) / (histogram.size());

		auto sorted_begin_it = sort.cbegin() + first;
		auto sorted_end_it = sort.cbegin() + last + 1;
		auto begin_it = core_array.cbegin() + first;
		auto end_it = core_array.cbegin() + last + 1;

		//#pragma omp parallel for firstprivate(sorted_begin_it, begin_it) //
		// working but slower
		for (size_t i = 0; i < histogram.size() - 1; i++) {
			const T value = (T)(min + (interval * (i + 1)));
			if (sort) {
				sorted_begin_it = std::lower_bound(
				    sorted_begin_it, sorted_end_it, 0,
				    [&core_array, &invalid_sel, value](size_t j, size_t) {
					    return core_array[j] < value and (not invalid_sel or not invalid_sel[j]);
				    });
				begin_it = core_array.cbegin() + std::distance(sort.cbegin(), sorted_begin_it);
			} else {
				begin_it = std::lower_bound(begin_it, end_it, value);
			}
			histogram[i] = std::distance(core_array.cbegin(), begin_it);
		}
		histogram.back() = last + 1;

		// Transform indexes into histogram
		size_t first_range = histogram.front() - first;
		std::adjacent_difference(histogram.begin(), histogram.end(), histogram.begin());
		histogram.front() = first_range;
	}

	array_impl_interface* subtract(const array_impl_interface* values,
	                               const array_impl_interface* groups) const override
	{
		const core::array<T>& core_array = *to_core_array();
		const core::array<T>& core_values = *values->to_core_array<T>();
		const core::array<index_t>& core_groups = *groups->to_core_array<index_t>();

		pvcop::types::formatter_interface::sp sformatter;

		if constexpr (std::is_same<T, uint64_t>::value) { // "datetime" or "datetime_ms"
			if (formatter()->name() == "datetime") {
				auto subtracted = primitive::subtract<boost::posix_time::time_duration>(
				    core_array, core_values, core_groups,
				    [](uint64_t v) { return boost::posix_time::seconds(v); });
				sformatter.reset(new pvcop::types::formatter_duration(""));
				return new array_impl<core::memarray, boost::posix_time::time_duration>(
				    std::move(subtracted), sformatter, dict(), invalid_dict());
			}
			else if (formatter()->name() == "datetime_ms") {
				auto subtracted = primitive::subtract<boost::posix_time::time_duration>(
				    core_array, core_values, core_groups,
				    [](uint64_t v) { return boost::posix_time::milliseconds(v); });
				sformatter.reset(new pvcop::types::formatter_duration(""));
				return new array_impl<core::memarray, boost::posix_time::time_duration>(
				    std::move(subtracted), sformatter, dict(), invalid_dict());
			}
		}

		if constexpr (std::is_same<T, boost::posix_time::ptime>::value) { // "datetime_us"
			auto subtracted = primitive::subtract<boost::posix_time::time_duration>(
			    core_array, core_values, core_groups);
			sformatter.reset(new pvcop::types::formatter_duration(""));
			return new array_impl<core::memarray, boost::posix_time::time_duration>(
			    std::move(subtracted), sformatter, dict(), invalid_dict());
		}

		using subtype_t = decltype(T() - T());
		auto subtracted = primitive::subtract<subtype_t>(core_array, core_values, core_groups);
		sformatter = __impl::get_proper_formatter<subtype_t>(formatter());
		return new array_impl<core::memarray, subtype_t>(std::move(subtracted), sformatter, dict(),
		                                                 invalid_dict());
	}

	void subselect(const array_impl_interface* values,
	               const db::selection& intput_sel,
	               db::selection& output_sel) const override
	{
		const core::array<T>& array = *to_core_array();
		const core::array<T>& values_native = *values->to_core_array<T>();

		// clear output selection
		std::fill(output_sel.begin(), output_sel.end(), false);

		if (not values) {
		    return;
		}

		primitive::subselect(core::make_selected_array(array, valid_selection(intput_sel)),
		                     core::make_selected_array(values_native, values->valid_selection()),
		                     output_sel);

		if (_invalid_selection) {
			primitive::subselect(
			    core::make_selected_array(array, invalid_selection(intput_sel)),
			    core::make_selected_array(values_native, values->invalid_selection()), output_sel);
		}
	}

	bool is_sorted() const override
	{
		const core::array<T>& array = *to_core_array();

		return std::is_sorted(array.begin(), array.end());
	}

	/**
	 * See db::array_base::parallel_sort documentation
	 */
	void parallel_sort(array_impl_interface* ind) const override
	{
		__impl::parallel_sorter<T>::parallel_sort(this, ind);
	}

	array_impl_interface* parallel_sort() const override
	{
		core::memarray<index_t> indexes(size());
		std::iota(indexes.begin(), indexes.end(), 0);

		array_impl_interface* ind = new array_impl<core::memarray, index_t>(
		    std::move(indexes),
		    types::formatter_interface::sp(new types::formatter_number<index_t>("")), nullptr,
		    invalid_dict(), invalid_selection());

		parallel_sort(ind);

		return ind;
	}

  private:
	CoreArrayBase<bool> _invalid_selection;
	mutable std::unique_ptr<array_impl_interface> _global_minmax;
};

} // namespace db

} // namespace pvcop

#include "db/array/primitive/sum.tpp"
#include "db/array/primitive/min_max.tpp"
#include "db/array/primitive/divide.tpp"
#include "db/array/primitive/group.tpp"
#include "db/array/primitive/group_distinct_count.tpp"
#include "db/array/primitive/group_count.tpp"
#include "db/array/primitive/group_sum.tpp"
#include "db/array/primitive/group_min.tpp"
#include "db/array/primitive/group_max.tpp"
#include "db/array/primitive/group_average.tpp"
#include "db/array/primitive/join.tpp"
#include "db/array/primitive/subselect.tpp"
#include "db/array/primitive/range_select.tpp"
#include "db/array/primitive/equal_range.tpp"
#include "db/array/primitive/subtract.tpp"

#endif // __PVCOP_DB_ARRAY_IMPL_H__
