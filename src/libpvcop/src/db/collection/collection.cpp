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

#include <pvcop/db/collection.h>
#include <pvcop/db/types.h>
#include <pvcop/db/read_dict.h>
#include <pvcop/db/write_dict.h>
#include <pvcop/db/exceptions/invalid_column.h>
#include <pvcop/db/exceptions/invalid_collection.h>

#include <db/collection/description.h>
#include <db/collection/file_read_handler.h>
#include <pvcop/db/types_manager.h>
#include <pvcop/utils/filesystem.h>

#include <exception>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <filesystem>

#include "pybind11/numpy.h"

/**
 * @class pvcop::db::collection
 *
 * @todo study the capability of changing the collection geometry (removing
 * column, adding new one, etc.). Must it keep track of retrieved columns
 * to refuse to remove them? It is doable as file_read_handler use a reference
 * counting mechanism to unmap files when not used anymore?
 */

/*****************************************************************************
 * pvcop::db::collection::collection
 *****************************************************************************/

pvcop::db::collection::collection(std::string const& rootdir) : _rootdir(rootdir)
{
	filesystem::sanitize_directory_path(_rootdir);

	if (check_access(_rootdir, filesystem::access_mode::WRITE, filesystem::entry_type::DIR) ==
	    false) {
		throw exception::invalid_collection("Can't access folder containing collection.");
	}

	description::process_header_f process_header = [&](size_t row_count, size_t column_count) {
		_row_count = row_count;
		_column_count = column_count;
		_handlers.resize(column_count);
		_dicts.resize(column_count);
	};


	description::process_column_f process_column =
	    [&](const std::string& data_path, const std::string& dict_path, size_t i, type_t t) {
		    if (t != "") {
			    _handlers[i] = std::make_unique<file_read_handler>(data_path, t, this->_row_count);
			    if (t == "string") {
				    _dicts[i] = std::make_unique<read_dict>(dict_path);
			    }
		    }
		};

	try {
		description::load(_rootdir, process_header, process_column);
	} catch (std::exception& e) {
		throw exception::invalid_collection(e.what());
	}
}

/*****************************************************************************
 * pvcop::db::collection::~collection
 *****************************************************************************/

pvcop::db::collection::~collection()
= default;

/*****************************************************************************
 * pvcop::db::collection::operator bool
 *****************************************************************************/

pvcop::db::collection::operator bool() const
{
	if (_handlers.size() == 0) {
		return false;
	}

	// check all file read handlers validity
	for (const auto& it : _handlers) {
		if (it == nullptr) {
			continue;
		}

		// Note: next line calls file_read_handler::operator bool()
		if (it->operator bool() == false) {
			return false;
		}
	}

	return true;
}

/*****************************************************************************
 * pvcop::db::collection::column
 *****************************************************************************/

pvcop::db::array pvcop::db::collection::column(size_t index) const
{

	if (index >= _handlers.size()) {
		throw pvcop::db::exception::invalid_column("Out of range column index : " +
		                                           std::to_string(index));
	}

	file_read_handler* frh = _handlers[index].get();

	assert(frh != nullptr && "Invalid file read handler.");

	const type_t t = frh->type();

	if (t == "") {
		throw pvcop::db::exception::invalid_column("No type defined for this column : " +
		                                           std::to_string(index));
	}

	/* we have a valid file_read_handler with a relevant type, it's time
	 * to get the file mapping
	 */
	void* base_address = frh->request();
	assert(base_address != nullptr && "Invalid base address");

	// frh is captured by value to make a copy in the lambda context
	auto rf = [frh]() { frh->release(); };

	const read_dict* dict = collection::dict(index);

	return pvcop::db::array(
	    types_manager::array_factory(t).create_filearray(base_address, _row_count, rf, dict));
}

/*****************************************************************************
 * pvcop::db::collection::dict
 *****************************************************************************/

const pvcop::db::read_dict* pvcop::db::collection::dict(size_t index) const
{
	if (index >= _handlers.size()) {
		throw pvcop::db::exception::invalid_column("Out of range column index : " +
		                                           std::to_string(index));
	}

	return _dicts[index].get();
}

/*****************************************************************************
 * pvcop::db::collection::type
 *****************************************************************************/

pvcop::db::type_t pvcop::db::collection::type(size_t index) const
{
	if (index >= _handlers.size()) {
		throw pvcop::db::exception::invalid_column("Out of range column index : " +
		                                           std::to_string(index));
	}

	return _handlers[index]->type();
}

/*****************************************************************************
 * pvcop::db::collection::append_column
 *****************************************************************************/

bool pvcop::db::collection::append_column(const pvcop::db::type_t& column_type, const pybind11::array& column)
{
	// write column to disk
	const std::string& column_path = _rootdir + std::to_string(_column_count);
	const std::string& column_data_path = column_path + description::data_file_extension;
	const std::string& column_dict_path = column_path + description::dict_file_extension;

	std::unique_ptr<db::read_dict> read_dict;
	bool ret = true;
	void const * data_ptr = nullptr;
	size_t bytes_count = 0;
	std::vector<db::write_dict::index_type> string_values;
	if (column_type == "string") {
		db::write_dict write_dict;
		string_values = deduplicate_strings(column, write_dict);
		write_dict.save(column_dict_path);
		read_dict = std::make_unique<db::read_dict>(column_dict_path);
		data_ptr = (void*)string_values.data();
		bytes_count = column.size() * sizeof(db::write_dict::index_type);
	}
	else {
		data_ptr = const_cast<pybind11::array&>(column).request().ptr; // safe
		bytes_count = types_manager::traits(column_type).to_mem_size(column.size());
	}
	
	auto column_file = std::fstream(std::filesystem::path(column_data_path), std::ios::out | std::ios::binary);
	column_file.write((const char*)data_ptr, bytes_count);
	column_file.close();
	ret = not column_file.bad();

	if (ret) {
		_column_count++;
		_handlers.emplace_back(new file_read_handler(column_data_path, column_type, _row_count));
		_dicts.emplace_back(std::move(read_dict));

		description::get_column_info_f get = [&](size_t i) {
			if (not this->_handlers[i]) {
				throw pvcop::db::exception::invalid_column("No valid handler for column : " +
														std::to_string(i));
			}
			return this->_handlers[i]->type();
		};

		description::append_column(_rootdir, _column_count, get); // update "description" file
	}

	return ret;
}

/*****************************************************************************
 * pvcop::db::collection::delete_column
 *****************************************************************************/

void pvcop::db::collection::delete_column(size_t column_index)
{
	// delete column from disk
	const std::string& column_path = _rootdir + std::to_string(column_index);
	const std::string& column_data_path = column_path + description::data_file_extension;
	const std::string& column_dict_path = column_path + description::dict_file_extension;
	const std::string& column_invalid_sel_path = column_path + description::invalid_sel_file_extension;

	remove(column_data_path.c_str());
	remove(column_dict_path.c_str());
	remove(column_invalid_sel_path.c_str());

	_column_count--;
	_handlers.erase(_handlers.begin() + column_index);
	_dicts.erase(_dicts.begin() + column_index);
	description::remove_column(_rootdir, column_index); // update "description" file
}

/*****************************************************************************
 * pvcop::db::collection::deduplicate_strings
 *****************************************************************************/

std::vector<pvcop::db::write_dict::index_type> pvcop::db::collection::deduplicate_strings(const pybind11::array& column, db::write_dict& write_dict)
{
	// TODO : parallelize ?
	std::vector<db::write_dict::index_type> values;
	values.reserve(column.size());
	PyObject* column_obj = column.ptr();
	for (size_t i = 0; i < (size_t) column.size(); i++) {
		std::unique_ptr<void, decltype(&PyMem_Free)> string((void*)PyUnicode_AsUTF8(PySequence_GetItem(column_obj, i)), &PyMem_Free);
		values.emplace_back(write_dict.insert((const char*)string.get()));
	}
	
	return values;
}

/*****************************************************************************
 * pvcop::db::collection::save_description
 *****************************************************************************/

void pvcop::db::collection::save_description()
{
	description::get_column_info_f get = [&](size_t i) {
		if (not this->_handlers[i]) {
			throw pvcop::db::exception::invalid_column("No valid handler for column : " +
			                                           std::to_string(i));
		}
		return this->_handlers[i]->type();
	};

	description::save(_rootdir, _row_count, _column_count, get);
}

/*****************************************************************************
 * pvcop::db::collection::close
 *****************************************************************************/

void pvcop::db::collection::close()
{
	save_description();

	/* enough to free everythings.
	 */
	_handlers.clear();
	_dicts.clear();
}
