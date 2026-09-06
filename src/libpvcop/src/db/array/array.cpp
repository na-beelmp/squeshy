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

#include <pvcop/db/array.h>
#include <pvcop/db/types.h>

#include <pvcop/db/exceptions/no_pimpl_error.h>

#include <pvcop/db/array_impl_interface.h>
#include "array_impl.h"
#include <pvcop/db/read_dict.h>
#include <pvcop/db/string_index_types.h>
#include <pvcop/types/string.h>
#include <pvlogger.h>

#include <iostream>
#include <memory>
#include <unordered_map>

namespace pvcop::db
{

/******************************************************************************
 *
 * pvcop::db::array
 *
 *****************************************************************************/

pvcop::db::array::array(array_impl_interface* pimpl) : array_base(pimpl)
{
}

pvcop::db::array::array(type_t type, size_t count /*= 0*/, bool init /*= false*/)
    : array_base(type, count, init)
{
}

pvcop::db::array::array(const array_impl_interface* pimpl, size_t pos, size_t len)
    : array_base(pimpl, pos, len)
{
}

pvcop::db::array::array() : array_base()
{
}

pvcop::db::array::~array()
= default;

pvcop::db::array pvcop::db::array::slice(size_t pos, size_t len) const
{
	pvlogger::debug() << "db::array::" << pvlogger::font::bold(__func__) << "(" << pos << ", "
	                  << len << ")" << std::endl;

	check_slice_params(pos, len, size(), __func__);

	return db::array(_pimpl->slice(pos, len));
}

pvcop::db::array pvcop::db::array::concat(const array& b) const
{
	pvlogger::debug() << "db::array::" << pvlogger::font::bold(__func__) << "(const db::array&)"
	                  << std::endl;

	if (not b) {
		if (_pimpl) {
			return db::array(_pimpl.get());
		} else {
			return {};
		}
	} else {
		if (not _pimpl) {
			return db::array(b._pimpl.get());
		}
	}

	if (type() != b.type()) {
		std::runtime_error("'db::array::concat': mismatching types");
	}

	if (std::string(formatter()->name()) != std::string(b.formatter()->name())) {
		std::runtime_error("'db::array::concat': mismatching formatters");
	}

	return db::array(_pimpl->concat(b._pimpl.get()));
}

pvcop::db::array pvcop::db::array::concat(const std::vector<db::array>& arrays)
{
	if (arrays.size() == 0) {
		return {};
	} else if (arrays.size() == 1) {
		return arrays[0].copy();
	} else {
		bool same_types = std::all_of(arrays.begin(), arrays.end(), [&](const db::array& a) {
			return a.type() == arrays[0].type();
		});

		if (not same_types) {
			throw std::runtime_error("'db::array::concat': mismatching types");
		}

		std::vector<array_impl_interface*> arrays_pimpl(arrays.size());
		std::transform(arrays.begin(), arrays.end(), arrays_pimpl.begin(),
		               [](const db::array& a) { return a._pimpl.get(); });
		return db::array(arrays[0]._pimpl->concat(arrays_pimpl));
	}
}

db::array pvcop::db::array::copy() const
{
	pvlogger::debug() << "db::array::" << pvlogger::font::bold(__func__) << "()" << std::endl;

	if (not _pimpl) {
		return {};
	}

	return db::array(_pimpl->copy());
}

pvcop::types::formatter_interface::shared_ptr pvcop::db::array::formatter() const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("get formatter called but no pimpl is set");
	}

	return _pimpl->formatter();
}

void pvcop::db::array::set_formatter(const pvcop::types::formatter_interface::shared_ptr fi)
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("set formatter called but no pimpl is set");
	}

	_pimpl->set_formatter(fi);
}

int pvcop::db::array::at(size_t i, char* str, const size_t str_len) const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'at' called while no pimpl is set");
	}

	return _pimpl->at(i, str, str_len);
}

std::string
pvcop::db::array::at(size_t index,
                     size_t str_len /*= types::formatter_interface::MAX_STRING_LENGTH */) const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'at' called but no pimpl is set");
	}

	std::vector<char> vec_str(str_len);
	char* str = vec_str.data();
	int written_size = _pimpl->at(index, str, str_len);
	if (written_size >= (int)str_len) {
		std::unique_ptr<char[]> long_str(new char[written_size + 1]);
		_pimpl->at(index, long_str.get(), written_size + 1);
		return std::string(long_str.get(), written_size);
	} else if (written_size > 0) {
		return std::string(str, written_size);
	}

	return {};
}

size_t pvcop::db::array::max_string_length() const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'maxlen' called while no pimpl is set");
	}

	return _pimpl->max_string_length();
}

const read_dict* pvcop::db::array::dict() const
{
	if (not _pimpl) {
		throw db::exception::no_pimpl_error("'dict' called while no pimpl is set");
	}

	return _pimpl->dict();
}

/******************************************************************************
 *
 * pvcop::db::indexes
 *
 *****************************************************************************/
pvcop::db::indexes::indexes() : db::array_base()
{
}

pvcop::db::indexes::indexes(size_t count) : db::array_base(type_index, count)
{
}

pvcop::db::indexes::indexes(array_impl_interface* pimpl) : array_base(pimpl)
{
}

pvcop::db::indexes& pvcop::db::indexes::operator=(indexes&& rhs)
{
	pvlogger::trace() << "db::indexes::operator=(array&&)" << std::endl;

	_pimpl.swap(rhs._pimpl);

	return *this;
}

indexes::~indexes()
= default;

const core::array<pvcop::db::indexes::type>& pvcop::db::indexes::to_core_array() const&
{
	return *_pimpl->to_core_array<type>();
}

core::array<pvcop::db::indexes::type>& pvcop::db::indexes::to_core_array()&
{
	return *_pimpl->to_core_array<type>();
}

pvcop::db::indexes pvcop::db::indexes::slice(size_t pos, size_t len) const
{
	pvlogger::debug() << "db::indexes::" << pvlogger::font::bold(__func__) << "(" << pos << ", "
	                  << len << ")" << std::endl;

	check_slice_params(pos, len, size(), __func__);

	return pvcop::db::indexes(_pimpl->slice(pos, len));
}

pvcop::db::indexes pvcop::db::indexes::concat(const db::indexes& b) const
{
	pvlogger::debug() << "db::indexes::" << pvlogger::font::bold(__func__) << "(const db::indexes&)"
	                  << std::endl;

	return pvcop::db::indexes(_pimpl->concat(b._pimpl.get()));
}

/******************************************************************************
 *
 * pvcop::db::groups
 *
 *****************************************************************************/
pvcop::db::groups::groups() : db::indexes()
{
}

pvcop::db::groups::groups(array_impl_interface* pimpl) : indexes(pimpl)
{
}

groups::~groups()
= default;

pvcop::db::groups pvcop::db::groups::slice(size_t pos, size_t len) const
{
	pvlogger::debug() << "db::groups::" << pvlogger::font::bold(__func__) << "(" << pos << ", "
	                  << len << ")" << std::endl;

	check_slice_params(pos, len, size(), __func__);

	return db::groups(_pimpl->slice(pos, len));
}

pvcop::db::groups pvcop::db::groups::concat(const db::groups& b) const
{
	pvlogger::debug() << "db::groups::" << pvlogger::font::bold(__func__) << "(const db::groups&)"
	                  << std::endl;

	return db::groups(_pimpl->concat(b._pimpl.get()));
}

/******************************************************************************
 *
 * pvcop::db::extents
 *
 *****************************************************************************/
pvcop::db::extents::extents() : db::indexes()
{
	pvlogger::trace() << "db::extents::" << pvlogger::font::bold(__func__) << std::endl;
}

pvcop::db::extents::extents(array_impl_interface* pimpl) : indexes(pimpl)
{
}

extents::~extents()
= default;

pvcop::db::extents pvcop::db::extents::slice(size_t pos, size_t len) const
{
	pvlogger::debug() << "db::extents::" << pvlogger::font::bold(__func__) << "(" << pos << ", "
	                  << len << ")" << std::endl;

	check_slice_params(pos, len, size(), __func__);

	return db::extents(_pimpl->slice(pos, len));
}

pvcop::db::extents pvcop::db::extents::concat(const db::extents& b) const
{
	pvlogger::debug() << "db::extents::" << pvlogger::font::bold(__func__) << "(const db::extents&)"
	                  << std::endl;

	return db::extents(_pimpl->concat(b._pimpl.get()));
}

array make_string_array(const std::vector<std::string>& values)
{
	// An empty array holds no value to look up, so it needs no dictionary --
	// and read_dict rejects an empty string set.
	if (values.empty()) {
		return db::array("string", 0);
	}

	// Dictionary first: read_dict takes the index of each string in the set as
	// its internal index, so the order built here is the one the array's
	// indices refer to.
	std::vector<std::string> dictionary;
	std::unordered_map<std::string, db::index_t> seen;
	dictionary.reserve(values.size());
	seen.reserve(values.size());

	std::vector<db::index_t> indices;
	indices.reserve(values.size());
	for (const std::string& value : values) {
		const auto it = seen.find(value);
		if (it != seen.end()) {
			indices.push_back(it->second);
			continue;
		}
		const auto index = db::index_t(dictionary.size());
		dictionary.push_back(value);
		seen.emplace(value, index);
		indices.push_back(index);
	}

	db::array array("string", values.size());

	// to_core_array() dereferences the storage, which an empty array does not
	// have; the dictionary is still attached below so the result is a usable,
	// merely empty, string array.
	if (not values.empty()) {
		auto& storage = array.to_core_array<string_index_t>();
		for (size_t i = 0; i < indices.size(); ++i) {
			storage[i] = string_index_t(indices[i]);
		}
	}

	// The formatter owns the dictionary, so the array stays usable on its own:
	// the formatter is shared with it and outlives every read.
	auto formatter = array.formatter();
	static_cast<types::formatter_string*>(formatter.get())
	    ->set_owned_read_dict(std::make_unique<db::read_dict>(dictionary));

	return array;
}

} // namespace pvcop
