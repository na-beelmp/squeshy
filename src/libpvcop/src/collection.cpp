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

#include <pvcop/collection.h>

#include <pvcop/types/factory.h>
#include <pvcop/types/string.h>
#include <pvcop/types/exception/unknown_formatter_error.h>
#include <pvcop/db/exceptions/invalid_collection.h>
#include <pvcop/db/exceptions/invalid_column.h>
#include <pvcop/db/types_manager.h>
#include <db/collection/description.h>
#include <sel_read_handler.h>

#include <fstream>
#include <memory>
#include <filesystem>

/*****************************************************************************
 * pvcop::collection::collection
 *****************************************************************************/

pvcop::collection::collection(std::string const& rootdir) : pvcop::db::collection(rootdir)
{
	try {
		_formatter_descs.load(db::collection::rootdir() + formatter_desc_list::filename);
	} catch (std::exception& e) {
		throw db::exception::invalid_collection(e.what());
	}

	_invalid_sel_handlers.resize(_column_count);
	_invalid_dicts.resize(_column_count);

	bool has_error = false;
	std::string exception_msg = "unknown formatters:";

	for (size_t i = 0; i < _formatter_descs.size(); ++i) {
		const auto& fd = _formatter_descs[i];

		if (not types::factory::is_registered(fd.name())) {
			has_error = true;
			exception_msg += " column " + std::to_string(i) + " (" + fd.name() + ")";
		}

		types::formatter_interface* fi = types::factory::create(fd.name(), fd.parameters());

		if (fi->name() == "string") {
			static_cast<types::formatter_string*>(fi)->set_read_dict(dict(i));
		}

		const std::string& invalid_sel_path =
		    _rootdir + std::to_string(i) + pvcop::db::description::invalid_sel_file_extension;
		if (std::ifstream(std::filesystem::path(invalid_sel_path)).good()) {
			_invalid_sel_handlers[i] = std::make_unique<sel_read_handler>(invalid_sel_path, _row_count);

			// invalid dicts with only an empty string are not stored to disk neither used
			// afterwards
			const std::string& invalid_dict_path =
			    _rootdir + std::to_string(i) + pvcop::db::description::invalid_dict_file_extension;
			if (std::ifstream(std::filesystem::path(invalid_dict_path)).good()) {
				_invalid_dicts[i] = std::make_unique<db::read_dict>(invalid_dict_path);
			}
		}

		_formatters.emplace_back(fi);
	}

	if (has_error) {
		_formatters.clear();
		throw types::exception::unknown_formatter_error(exception_msg);
	}
}

pvcop::collection::~collection() = default;

/*****************************************************************************
 * pvcop::collection::column
 *****************************************************************************/

pvcop::db::array pvcop::collection::column(size_t index) const
{
	assert(_handlers.size() == _invalid_sel_handlers.size());
	if (index >= _handlers.size()) {
		throw pvcop::db::exception::invalid_column("Out of range column index : " +
		                                           std::to_string(index));
	}

	db::file_read_handler* data_rh = _handlers[index].get();
	sel_read_handler* invalid_sel_frh = _invalid_sel_handlers[index].get();

	assert(data_rh != nullptr && "Invalid file read handler.");

	const db::type_t t = data_rh->type();

	if (t == "") {
		throw pvcop::db::exception::invalid_column("No type defined for this column : " +
		                                           std::to_string(index));
	}

	/* we have a valid file_read_handler with a relevant type, it's time
	 * to get the file mapping
	 */
	void* data_base_address = data_rh->request();
	assert(data_base_address != nullptr && "Invalid base address");

	void* invalid_sel_base_address = invalid_sel_frh ? invalid_sel_frh->request() : nullptr;

	// frh is captured by value to make a copy in the lambda context
	auto data_rf = [data_rh]() { data_rh->release(); };
	auto invalid_sel_rf = [invalid_sel_frh]() {
		if (invalid_sel_frh)
			invalid_sel_frh->release();
	};

	const db::read_dict* dict = collection::dict(index);

	pvcop::db::array array(db::types_manager::array_factory(t).create_filearray(
	    data_base_address, _row_count, data_rf, dict, types::formatter_interface::shared_ptr(),
	    invalid_sel_base_address, invalid_sel_rf, _invalid_dicts[index].get()));

	array.set_formatter(_formatters[index]);

	return array;
}

/*****************************************************************************
 * pvcop::collection::append_column
 *****************************************************************************/
bool pvcop::collection::append_column(const pvcop::db::type_t& column_type, const pybind11::array& column)
{
	bool ret = pvcop::db::collection::append_column(column_type, column);
	
	if (ret) {
		_formatters.emplace_back(types::factory::create(column_type, ""));
		_invalid_sel_handlers.emplace_back(nullptr);
		_invalid_dicts.emplace_back(nullptr);

		// update "formatters" file
		_formatter_descs.emplace_back(_formatters.back()->name(), "");
		if (column_type == "string") {
			types::formatter_interface* fi = _formatters.back().get();
			((types::formatter_string*)fi)->set_read_dict(_dicts.back().get());
		}
		_formatter_descs.save(_rootdir + formatter_desc_list::filename);
	}
	
	return ret;
}

/*****************************************************************************
 * pvcop::collection::delete_column
 *****************************************************************************/

void pvcop::collection::delete_column(size_t column_index)
{
	assert(column_index < pvcop::db::collection::column_count());

	pvcop::db::collection::delete_column(column_index);

	_formatter_descs.erase(_formatter_descs.begin() + column_index);
	_formatters.erase(_formatters.begin() + column_index);
	_invalid_sel_handlers.erase(_invalid_sel_handlers.begin() + column_index);
	_invalid_dicts.erase(_invalid_dicts.begin() + column_index);
	_formatter_descs.save(_rootdir + formatter_desc_list::filename);
}

/*****************************************************************************
 * pvcop::collection::operator bool()
 *****************************************************************************/

pvcop::collection::operator bool() const
{
	return db::collection::operator bool() && (column_count() == _formatters.size());
}
