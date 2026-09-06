//
// MIT License
//
// © ESI Group, 2015
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <pvcop/db/array_base.h>

#include <pvcop/core/array.h>
#include <pvcop/db/array.h>
#include <pvcop/db/exceptions/no_pimpl_error.h>
#include <pvcop/db/exceptions/array_type_error.h>
#include <pvcop/types/impl/formatter_factory.h>

#include <pvlogger.h>

#include <pvcop/db/types_manager.h>

#include <iostream>

namespace pvcop::db
{

pvcop::db::array_base::array_base(array_impl_interface* pimpl) : _pimpl(pimpl)
{
	pvlogger::trace() << "db::array_base::array_base(pimpl)" << std::endl;
}

pvcop::db::array_base::array_base(type_t type, size_t count /*= 0*/, bool init /* = false */)
    : _pimpl(types_manager::array_factory(type).create_memarray(
          count,
          pvcop::types::formatter_interface::shared_ptr(
              types::__impl::formatter_factory::create(type, "")),
          init))
{
	pvlogger::trace() << "db::array_base::array_base(" << type << ", " << count << ")" << std::endl;
}

pvcop::db::array_base::array_base(array_base&& rhs) : _pimpl(std::move(rhs._pimpl))
{
	rhs._pimpl.reset();

	pvlogger::trace() << "db::array_base::array_base(array_base&&)" << std::endl;
}

pvcop::db::array_base::array_base() : _pimpl(nullptr)
{
	pvlogger::trace() << "db::array_base::array_base()" << std::endl;
}

pvcop::db::array_base::array_base(const array_impl_interface* pimpl, size_t pos, size_t len)
    : _pimpl(types_manager::array_factory(pimpl->type()).create_array(pimpl, pos, len))
{
	pvlogger::trace() << "db::array_base::array_base(" << pos << ", " << len << ")" << std::endl;
}

array_base& pvcop::db::array_base::operator=(array_base&& rhs)
{
	pvlogger::trace() << "db::array_base::operator=(array&&)" << std::endl;

	_pimpl.swap(rhs._pimpl);

	return *this;
}

pvcop::db::array_base::~array_base()
{
	pvlogger::trace() << "db::array_base::~array_base()" << std::endl;
}

bool pvcop::db::array_base::operator==(const pvcop::db::array_base& other) const
{
	return *_pimpl == *other._pimpl;
}

bool pvcop::db::array_base::operator!=(const pvcop::db::array_base& other) const
{
	return !(*this == other);
}

size_t pvcop::db::array_base::size() const
{
	if (not _pimpl) {
		return 0;
	}

	return _pimpl->size();
}

const void* pvcop::db::array_base::data() const
{
	if (not _pimpl) {
		return nullptr;
	}

	return _pimpl->data();
}

void* pvcop::db::array_base::data()
{
	return const_cast<void*>(static_cast<const array_base*>(this)->data());
}

pvcop::db::type_t pvcop::db::array_base::type() const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'type' called but no pimpl is set");
	}

	return _pimpl->type();
}

bool pvcop::db::array_base::is_string() const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'is_string' called but no pimpl is set");
	}

	return _pimpl->type() == "string";
}

db::INVALID_TYPE pvcop::db::array_base::has_invalid() const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'has_invalid' called but no pimpl is set");
	}

	return _pimpl->has_invalid();
}

bool pvcop::db::array_base::is_valid(size_t row) const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'is_valid' called but no pimpl is set");
	}

	return _pimpl->is_valid(row);
}

pvcop::core::memarray<bool>
pvcop::db::array_base::valid_selection(const db::selection& sel /* = {} */) const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'is_valid' called but no pimpl is set");
	}

	return _pimpl->valid_selection(sel);
}

pvcop::core::memarray<bool> pvcop::db::array_base::invalid_selection(const db::selection& sel) const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'invalid_selection' called but no pimpl is set");
	}

	return _pimpl->invalid_selection(sel);
}

pvcop::core::selection pvcop::db::array_base::invalid_selection() const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'invalid_selection' called but no pimpl is set");
	}

	return _pimpl->invalid_selection();
}

size_t pvcop::db::array_base::valid_count(const db::selection& sel) const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'valid_count' called but no pimpl is set");
	}

	return _pimpl->valid_count(sel);
}

size_t pvcop::db::array_base::invalid_count(const db::selection& sel) const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'invalid_count' called but no pimpl is set");
	}

	return _pimpl->invalid_count(sel);
}

db::array
pvcop::db::array_base::to_array(std::vector<std::string>::const_iterator begin,
                                std::vector<std::string>::const_iterator end,
                                std::vector<std::string>* unconvertable_values /*= nullptr*/) const
{
	return db::array(_pimpl->to_array(begin, end, unconvertable_values));
}

db::array
pvcop::db::array_base::to_array(const std::vector<std::string>& strings,
                                std::vector<std::string>* unconvertable_values /*= nullptr*/) const
{
	return to_array(strings.begin(), strings.end(), unconvertable_values);
}

db::array pvcop::db::array_base::to_array_if(
    std::vector<std::string>::const_iterator begin,
    std::vector<std::string>::const_iterator end,
    std::function<bool(const std::string& v, const std::string& e)> f) const
{
	return db::array(_pimpl->to_array_if(begin, end, f));
}

bool pvcop::db::array_base::is_sorted() const
{
	pvlogger::debug() << "db::array::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'is_sorted' called while no pimpl is set");
	}

	// Revert argument order as we need to know sorted array type.
	return _pimpl->is_sorted();
}

db::indexes pvcop::db::array_base::parallel_sort() const
{
	pvlogger::debug() << "db::array::" << pvlogger::font::bold(__func__) << "(db::indexes& indexes)"
	                  << std::endl;

	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'parallel_sort' called while no pimpl is set");
	}

	// Revert argument order as we need to know sorted array type.
	return pvcop::db::indexes(_pimpl->parallel_sort());
}

void pvcop::db::array_base::parallel_sort(db::indexes& ind) const
{
	pvlogger::debug() << "db::array::" << pvlogger::font::bold(__func__) << "(db::indexes& indexes)"
	                  << std::endl;

	if (not _pimpl or not ind._pimpl) {
		throw db::exception::no_pimpl_error("'parallel_sort' called while no pimpl is set");
	}

	if (ind.size() > size()) {
		throw db::exception::size_mismatch_error(
		    "db::indexes::parallel_sort_on(...): provided array has an incorrect size.");
	}

	// Revert argument order as we need to know sorted array type.
	_pimpl->parallel_sort(ind._pimpl.get());
}

db::array pvcop::db::array_base::sum(const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (not _pimpl) {
		pvlogger::error() << "db::array_base::" << pvlogger::font::bold(__func__)
		                  << ": invalid array" << std::endl;
		return {};
	}

	return db::array(_pimpl->sum(sel));
}

db::array pvcop::db::array_base::min(const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (not _pimpl) {
		pvlogger::error() << "db::array_base::" << pvlogger::font::bold(__func__)
		                  << ": invalid array" << std::endl;
		return {};
	}

	return db::array(_pimpl->min(sel));
}

db::array pvcop::db::array_base::max(const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (not _pimpl) {
		pvlogger::error() << "db::array_base::" << pvlogger::font::bold(__func__)
		                  << ": invalid array" << std::endl;
		return {};
	}

	return db::array(_pimpl->max(sel));
}

db::array pvcop::db::array_base::minmax(const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (not _pimpl) {
		pvlogger::error() << "db::array_base::" << pvlogger::font::bold(__func__)
		                  << ": invalid array" << std::endl;
		return {};
	}

	return db::array(_pimpl->minmax(sel));
}

db::array pvcop::db::array_base::divide(const db::array& divisors,
                                        const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (divisors.size() > size()) {
		throw db::exception::size_mismatch_error(
		    "db::indexes::divide(...): provided array has an incorrect size.");
	}

	return db::array(_pimpl->divide(divisors._pimpl.get(), sel));
}

db::array pvcop::db::array_base::average(const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (not _pimpl) {
		pvlogger::error() << "db::array_base::" << pvlogger::font::bold(__func__)
		                  << ": invalid array" << std::endl;
		return {};
	}

	return db::array(_pimpl->average(sel));
}

void pvcop::db::array_base::group(db::groups& groups,
                                  db::extents& extents,
                                  const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(db::groups*, db::extents*)" << std::endl;

	if (groups) {
		pvlogger::warn() << pvlogger::font::bold(__func__)
		                 << ": 'groups' parameter is a valid array" << std::endl;
	}

	if (extents) {
		pvlogger::warn() << pvlogger::font::bold(__func__)
		                 << ": 'extents' parameter is a valid array" << std::endl;
	}

	_pimpl->group(groups._pimpl, extents._pimpl, sel);
}

db::array pvcop::db::array_base::group_sum(const db::groups& groups,
                                           const db::extents& extents,
                                           const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::groups&, const db::extents&)" << std::endl;

	check_group_operation_params(groups, extents, __func__);

	return db::array(_pimpl->group_sum(groups._pimpl.get(), extents._pimpl.get(), sel));
}

db::array pvcop::db::array_base::group_count(const db::groups& groups,
                                             const db::extents& extents,
                                             const db::selection&) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold("group_count")
	                  << "(const db::groups&, const db::extents&)" << std::endl;

	check_group_operation_params(groups, extents, __func__);

	return db::array(_pimpl->group_count(groups._pimpl.get(), extents._pimpl.get()));
}

db::array pvcop::db::array_base::group_distinct_count(const db::groups& groups,
                                                      const db::extents& extents,
                                                      const db::selection& sel) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::groups&, const db::extents&)" << std::endl;

	check_group_operation_params(groups, extents, __func__);

	return db::array(_pimpl->group_distinct_count(groups._pimpl.get(), extents._pimpl.get(), sel));
}

db::array pvcop::db::array_base::group_min(const db::groups& groups,
                                           const db::extents& extents,
                                           const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::groups&, const db::extents&)" << std::endl;

	check_group_operation_params(groups, extents, __func__);

	return db::array(_pimpl->group_min(groups._pimpl.get(), extents._pimpl.get(), sel));
}

db::array pvcop::db::array_base::group_max(const db::groups& groups,
                                           const db::extents& extents,
                                           const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::groups&, const db::extents&)" << std::endl;

	check_group_operation_params(groups, extents, __func__);

	return db::array(_pimpl->group_max(groups._pimpl.get(), extents._pimpl.get(), sel));
}

db::array pvcop::db::array_base::group_average(const db::groups& groups,
                                               const db::extents& extents,
                                               const db::selection& sel /*= db::selection()*/) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::groups&, const db::extents&)" << std::endl;

	check_group_operation_params(groups, extents, __func__);

	return db::array(_pimpl->group_average(groups._pimpl.get(), extents._pimpl.get(), sel));
}

/******************************************************************************
 *
 * pvcop::db::indexes::join
 *
 *****************************************************************************/
db::array pvcop::db::array_base::join(const db::indexes& a2) const
{
	pvlogger::debug() << "db::indexes::" << pvlogger::font::bold(__func__) << "(const db::array&)"
	                  << std::endl;

	return db::array(_pimpl->join(a2._pimpl.get()));
}

db::array pvcop::db::array_base::join(const db::selection& sel) const
{
	pvlogger::debug() << "db::indexes::" << pvlogger::font::bold(__func__) << "(const db::array&)"
	                  << std::endl;

	return db::array(_pimpl->join(sel));
}

/*****************************************************************************
 *
 * range_select
 *
 ****************************************************************************/
void pvcop::db::array_base::range_select(const std::string& min,
                                         const std::string& max,
                                         const db::selection& input_sel,
                                         db::selection& output_sel) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::array& values)" << std::endl;

	_pimpl->range_select(min, max, input_sel, output_sel);
}

void pvcop::db::array_base::range_select(const std::string& min,
                                         const std::string& max,
                                         const db::range_t& input_range,
                                         db::selection& output_sel) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::array& values)" << std::endl;

	_pimpl->range_select(min, max, input_range, output_sel);
}

/*****************************************************************************
 *
 * minmax_range
 *
 ****************************************************************************/
db::range_t pvcop::db::array_base::equal_range(const db::array& minmax) const
{
	return equal_range(minmax, {});
}

db::range_t pvcop::db::array_base::equal_range(const db::array& minmax,
                                               const db::indexes& sort_order) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::array& values)" << std::endl;

	if (minmax.size() != 2) {
		throw db::exception::size_mismatch_error("minmax array size should be 2");
	}

	if (sort_order and sort_order.size() != size()) {
		throw db::exception::size_mismatch_error(
		    "sort_order array size should be equal to array size");
	}

	if (type() != minmax.type()) {
		throw db::exception::array_type_error("minmax array is not of the expected type");
	}

	return _pimpl->equal_range(minmax._pimpl.get(), sort_order._pimpl.get());
}

db::array pvcop::db::array_base::ratio_to_minmax(double ratio1, double ratio2) const
{
	return ratio_to_minmax(ratio1, ratio2, {});
}

db::array pvcop::db::array_base::ratio_to_minmax(double ratio1,
                                                 double ratio2,
                                                 const db::array& global_minmax /*= {}*/) const
{
	if (ratio1 > 1. or ratio1 < 0. or ratio2 > 1. or ratio2 < 0.) {
		throw db::exception::array_type_error("ratios should be in range [0..1]");
	}

	if (global_minmax and global_minmax.size() != 2) {
		throw db::exception::size_mismatch_error("global_minmax array size should be 2");
	}

	return db::array(_pimpl->ratio_to_minmax(ratio1, ratio2, global_minmax._pimpl.get()));
}

void pvcop::db::array_base::histogram(size_t first,
                                      size_t last,
                                      const db::array& minmax,
                                      const db::indexes& sort_order,
                                      std::vector<size_t>& histogram) const
{
	if (minmax.size() != 2) {
		throw db::exception::size_mismatch_error("minmax array size should be 2");
	}

	if (sort_order and sort_order.size() != size()) {
		throw db::exception::size_mismatch_error(
		    "sort_order array size should be equal to array size");
	}

	_pimpl->histogram(first, last, minmax._pimpl.get(), sort_order._pimpl.get(), histogram);
}

std::pair<double, double> pvcop::db::array_base::minmax_to_ratio(const db::array& minmax) const
{
	return minmax_to_ratio(minmax, {});
}

std::pair<double, double>
pvcop::db::array_base::minmax_to_ratio(const db::array& minmax,
                                       const db::array& global_minmax /*= {}*/) const
{
	if (minmax.size() != 2) {
		throw db::exception::size_mismatch_error("minmax array size should be 2");
	}

	if (global_minmax and global_minmax.size() != 2) {
		throw db::exception::size_mismatch_error("global_minmax array size should be 2");
	}

	if (type() != minmax.type()) {
		throw db::exception::array_type_error("minmax array is not of the expected type");
	}

	return _pimpl->minmax_to_ratio(minmax._pimpl.get(), global_minmax._pimpl.get());
}

db::array pvcop::db::array_base::subtract(const db::array& values, db::groups& groups) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::array& values)" << std::endl;

	if (not _pimpl) {
		pvlogger::error() << "db::array_base::" << pvlogger::font::bold(__func__)
		                  << ": invalid array" << std::endl;
		return {};
	}

	if (size() != groups.size()) {
		throw db::exception::size_mismatch_error("groups array size should match array size");
	}

	if (type() != values.type()) {
		throw db::exception::array_type_error("values array is not of the expected type");
	}

	return db::array(_pimpl->subtract(values._pimpl.get(), groups._pimpl.get()));
}

void pvcop::db::array_base::subselect(const db::array& search_array,
                                      const db::selection& input_sel,
                                      db::selection& output_sel) const
{
	pvlogger::debug() << "db::array_base::" << pvlogger::font::bold(__func__)
	                  << "(const db::array& values)" << std::endl;

	_pimpl->subselect(search_array._pimpl.get(), input_sel, output_sel);
}

void pvcop::db::array_base::check_slice_params(size_t pos,
                                               size_t len,
                                               size_t array_size,
                                               const char* func)
{
	if (pos >= array_size) {
		pvlogger::error() << pvlogger::font::bold(func) << ": starting position (" << pos << ")"
		                  << " exceeds array size (" << array_size << ")" << std::endl;
		throw std::runtime_error("FAIL");
	}

	if ((pos + len) > array_size) {
		pvlogger::warn() << pvlogger::font::bold(func) << ": length (" << len << ")"
		                 << " exceeds array size (" << array_size << ")" << std::endl;
		throw std::runtime_error("FAIL");
	}
}

void pvcop::db::array_base::check_group_operation_params(const db::groups& groups,
                                                         const db::extents& extents,
                                                         const char* func)
{
	if (not groups) {
		pvlogger::error() << pvlogger::font::bold(func)
		                  << ": 'groups' parameter is not a valid array" << std::endl;
		throw std::runtime_error("FAIL");
	}

	if (not extents) {
		pvlogger::error() << pvlogger::font::bold(func)
		                  << ": 'extents' parameter is not a valid array" << std::endl;
		throw std::runtime_error("FAIL");
	}
}

} // namespace pvcop
