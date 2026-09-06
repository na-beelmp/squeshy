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

#ifndef __PVCOP_DB_ARRAY_BASE_H__
#define __PVCOP_DB_ARRAY_BASE_H__

#include <pvcop/db/selection.h>
#include <pvcop/db/types.h>

#include <pvcop/db/exceptions/partially_converted_error.h>
#include <pvcop/db/exceptions/unable_to_convert_input_error.h>

#include <pvcop/core/memarray.h>
#include <pvcop/db/array_impl_interface.h>

#include <memory>
#include <string>
#include <vector>

namespace pvcop
{

namespace core
{
template <typename T>
class array;
}

namespace db
{

class array;
class indexes;
class groups;
class extents;

/******************************************************************************
 *
 * pvcop::db::array_base
 *
 *****************************************************************************/

/**
 * Base class of db::array intended to share the primitive forwarding methods among db::array
 *subtypes.
 *
 * This class is needed because db::array subtypes can't inherit db::array as their "slice" and
 *"append"
 * methods need to be redefined to return the exact subtype.
 */
class array_base
{
	friend class indexes;

  protected:
	/**
	 * invalid constructor
	 */
	array_base();

	/**
	 * initialization constructor
	 *
	 * @param type the type of the array as a string
	 * @param count the number of elements contained in the array
	 */
	array_base(type_t type, size_t count, bool init = false);

	/**
	 * slice constructor
	 *
	 * @param pimpl the original array pimpl
	 * @param pos the start position in @a array
	 * @param len the size of the new array
	 */
	array_base(const array_impl_interface* pimpl, size_t pos, size_t len);

	/**
	 * pimpl constructor
	 *
	 * @param pimpl the pimpl created by the array_factory
	 */
	explicit array_base(array_impl_interface* pimpl);

	/**
	 * No copy constructor
	 */
	array_base(const db::array_base& rhs) = delete;

	/**
	 * move constructor
	 *
	 * @param rhs the array to move from
	 */
	array_base(db::array_base&& rhs);

	/**
	 * No copy assignment operator
	 */
	db::array_base& operator=(const db::array_base& rhs) = delete;

	/**
	 * move assignment operator
	 *
	 * @param rhs the array to move assign from
	 */
	db::array_base& operator=(db::array_base&& rhs);

	/**
	 * destructor
	 */
	~array_base();

  public:
	/**
	 * access the core::array low level representation of the db::array.
	 *
	 * @param T the internal value_type of the stored data.
	 *
	 * @return the core::array low level representation
	 *         or an invalid array if the provided type T differs from the one in core::array.
	 */
	template <typename T>
	const core::array<T>& to_core_array() const&
	{
		return *_pimpl->to_core_array<T>();
	}

	template <typename T>
	core::array<T>& to_core_array() &
	{
		return const_cast<core::array<T>&>(
		    static_cast<const array_base*>(this)->to_core_array<T>());
	}

	template <typename T>
	core::array<T> to_core_array() &&
	{
		static_assert(not std::is_same<T, T>::value, "We should not use to_core_array on rvalue");
		return {};
	}

	/**
	 * conversion to bool to check the array validity
	 *
	 * @return @c true if the array is valid; @c false otherwise
	 */
	inline explicit operator bool() const { return (bool)_pimpl; }

	/**
	 * Compare the content of two arrays
	 *
	 * return true if the type, size and content are equals,
	 *        false otherwise.
	 */
	bool operator==(const array_base& other) const;
	bool operator!=(const array_base& other) const;

  public:
	/**
	 * retrieve array size
	 *
	 * @return the array size
	 */
	size_t size() const;

	/**
	 * retrieve array type as a string
	 *
	 * @return the array type
	 */
	type_t type() const;

	/**
	 * Helper function to determine if array type is 'string'
	 */
	bool is_string() const;

	void* data();
	const void* data() const;

  public:
	/**
	 * Check if this array has some invalid values
	 */
	db::INVALID_TYPE has_invalid() const;

	/**
	 * Check if the specified row is valid according to the underlying type of the array
	 */
	bool is_valid(size_t row) const;

	/**
	 * Return the selection associated with valid/invalid rows
	 *
	 * @param sel
	 */
	core::memarray<bool> valid_selection(const db::selection& sel = {}) const;
	core::memarray<bool> invalid_selection(const db::selection& sel) const;
	pvcop::core::selection invalid_selection() const;

	/**
	 * Return the valid/invalid element count given a selection
	 */
	size_t valid_count(const db::selection& sel = {}) const;
	size_t invalid_count(const db::selection& sel = {}) const;

  public:
	/**
	 * Returns true if all the values of the array are sorted
	 */
	bool is_sorted() const;

	/**
	 * Index-sort the content of the array and return the result
	 * as a db::indexes.
	 * This allows to sort multiple columns according to one column.
	 */
	db::indexes parallel_sort() const;

	/**
	 * Inplace-sort the provided db::indexes according to the content this array.
	 *
	 * @throw size_mismatch_error
	 */
	void parallel_sort(db::indexes& ind) const;

	db::array sum(const db::selection& sel = db::selection()) const;
	db::array average(const db::selection& sel = db::selection()) const;
	db::array min(const db::selection& sel = db::selection()) const;
	db::array max(const db::selection& sel = db::selection()) const;
	db::array minmax(const db::selection& sel = db::selection()) const;

	db::array divide(const db::array& divisors, const db::selection& sel = db::selection()) const;

	/**
	 * Create the group array (index to group mapping) and extents array (group to first occurrence)
	 *
	 * @note: the selection used to compute the group must be reused with the associated group_*
	 *operations
	 *
	 * FIXME : We should return multiple values instead :
	 *https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Rf-T-return
	 */
	void group(db::groups& groups,
	           db::extents& extents,
	           const db::selection& sel = db::selection()) const;

	/**
	 * Returns the count of distinct values for each group
	 *
	 * warning : this primitive should not be parallelized
	 */
	db::array group_distinct_count(const db::groups& groups,
	                               const db::extents& extents,
	                               const db::selection& sel = db::selection()) const;
	db::array group_count(const db::groups& groups,
	                      const db::extents& extents,
	                      const db::selection& sel = db::selection()) const;
	db::array group_sum(const db::groups& groups,
	                    const db::extents& extents,
	                    const db::selection& sel = db::selection()) const;
	db::array group_min(const db::groups& groups,
	                    const db::extents& extents,
	                    const db::selection& sel = db::selection()) const;
	db::array group_max(const db::groups& groups,
	                    const db::extents& extents,
	                    const db::selection& sel = db::selection()) const;
	db::array group_average(const db::groups& groups,
	                        const db::extents& extents,
	                        const db::selection& sel = db::selection()) const;

	/**
	 * Get selected values from indices.
	 *
	 * @see https://gitlab.srv.picviz/picviz/libpvcop/wikis/BAT#batleftfetchjoin
	 */
	db::array join(const db::indexes& array) const;
	db::array join(const db::selection& sel) const;

	/**
	 * Filter a selection with a min and max value.
	 *
	 * @param[in] min : Minimum possible value
	 * @param[in] max : Maximum possible value
	 * @param[in] input_sel input selection used to filter as well
	 * @param[out] output_sel the resulting selection
	 */
	void range_select(const std::string& min,
	                  const std::string& max,
	                  const db::selection& input_sel,
	                  db::selection& output_sel) const;

	void range_select(const std::string& min,
	                  const std::string& max,
	                  const db::range_t& input_range,
	                  db::selection& output_sel) const;

	/**
	 * Returns the range of values located in between a minimum and a maximum value
	 *
	 * @note The array is assumed to be sorted, otherwise the sort order should be provided
	 *
	 * @param[in] minmax : Array of two values containing the minimum and maximum values
	 * @param[in] sort_order : Indexes of the sort order
	 *
	 * @return the range of values
	 */
	db::range_t equal_range(const db::array& minmax) const;
	db::range_t equal_range(const db::array& minmax, const db::indexes& sort_order) const;

	/**
	 * Compute a local minmax range by applying a ratio over the global minmax range
	 *
	 * @param[in] ratio1 : ratio of the local min value (between 0. and 1.)
	 * @param[in] ratio2 : ratio of the local max value (between 0. and 1.)
	 * @param[in] global_minmax : global minmax range (recomputed if not provided)
	 *
	 * @return the local minmax range
	 */
	db::array ratio_to_minmax(double ratio1, double ratio2) const;
	db::array ratio_to_minmax(double ratio1, double ratio2, const db::array& global_minmax) const;

	/**
	 * Returns the ratio range of the local minmax range
	 *
	 * @param[in] minmax : local minmax range
	 *
	 * @return the local ratio range (between 0. and 1.)
	 */
	std::pair<double, double> minmax_to_ratio(const db::array& minmax) const;
	std::pair<double, double> minmax_to_ratio(const db::array& minmax,
	                                          const db::array& global_minmax) const;

	/**
	 * Compute the histogram of values between two indexes
	 *
	 * @param[in] first first index
	 * @param[in] last last index
	 * @param[in] minmax local minmax
	 * @param[in] sort_order order of the sort
	 * @param[out] histogram histogram of values
	 *
	 * @note The selection is not taken into account
	 */
	void histogram(size_t first,
	               size_t last,
	               const db::array& minmax,
	               const db::indexes& sort_order,
	               std::vector<size_t>& histogram) const;

	/**
	 * Subtract a value to each value of the array
	 *
	 * @param[in] values : values to subtract
	 * @param[in] groups : groups to select proper value to subtract
	 *
	 * @return the resulting array with values subtracted
	 */
	db::array subtract(const db::array& values, db::groups& groups) const;

	/**
	 * Filter a selection with a list of exact matching values
	 *
	 * @param[in] search_array the values to filter
	 * @param[in] input_sel input selection used to filter as well
	 * @param[out] output_sel the resulting selection
	 * @param[in] invalid_search_array the invalid values to filter
	 */
	void subselect(const db::array& search_array,
	               const db::selection& input_sel,
	               db::selection& output_sel) const;

	/**
	 * Convert a list of strings to their native representation
	 * (using iterators in order to be parallelized easily)
	 *
	 * @note this method could also be in the formatter or in a static method
	 * taking the formatter as a parameter
	 *
	 * @param[in] begin iterator to the first element of the string list to convert
	 * @param[in] end iterator to the end element of the string list to convert
	 * @param[out] unconvertable values (not in valid or invalid domain)
	 *
	 * @return db::array containing the converted values (valid and invalid)
	 */
	db::array to_array(std::vector<std::string>::const_iterator begin,
	                   std::vector<std::string>::const_iterator end,
	                   std::vector<std::string>* unconvertable_values = nullptr) const;

	db::array to_array(const std::vector<std::string>& strings,
	                   std::vector<std::string>* unconvertable_values = nullptr) const;

	/**
	 * Convert a list of strings to their native representations using a predicate
	 * to filter them
	 *
	 * @param[in] begin iterator to the first element of the string list to convert
	 * @param[in] end iterator to the end element of the string list to convert
	 * @param[in] custom_match_f function tacking 2 strings as parameters (value and expression).
	 * 							 must return true if value match the expression, false otherwise
	 *
	 * @return db::array containing the converted values
	 */
	db::array
	to_array_if(std::vector<std::string>::const_iterator begin,
	            std::vector<std::string>::const_iterator end,
	            std::function<bool(const std::string& value, const std::string& expression)>
	                custom_match_f = {}) const;

  protected:
	static void check_slice_params(size_t pos, size_t len, size_t array_size, const char* func);
	static void check_group_operation_params(const db::groups& groups,
	                                         const db::extents& extents,
	                                         const char* func);

  protected:
	std::unique_ptr<array_impl_interface> _pimpl;
};

} // namespace db

} // namespace pvcop

#endif // __PVCOP_DB_ARRAY_BASE_H__
