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

#include <pvcop/db/collector.h>
#include <pvcop/db/write_dict.h>

#include <db/collection/description.h>
#include <pvcop/db/file_write_handler.h>
#include <pvcop/db/exceptions/collector_error.h>
#include <pvcop/db/exceptions/invalid_column.h>

#include <pvcop/utils/filesystem.h>

#include <memory>
#include <fstream>
#include <exception>
#include <stdexcept>

#include <climits>
#include <cstring>

#include <unistd.h>

/**
 * @class pvcop::db::collector
 *
 * @note how to permit to open a collection in append mode?
 *
 * @todo must the sink be tracked?
 */

/*****************************************************************************
 * pvcop::db::collector::collector
 *****************************************************************************/

/**
 * @todo hardening the allocation/
 */
pvcop::db::collector::collector(const char* rootdir, const format& f)
    : _row_count(0), _column_count(0)
{
	std::string tmp_path = rootdir;

	/* check rootdir validity & existence
	 */
	if (not check_access(tmp_path, filesystem::access_mode::WRITE, filesystem::entry_type::DIR)) {
		if (not filesystem::make_path(rootdir)) {
			throw exception::collector_error("Fail to get rootdir of collector :" +
			                                 std::string(rootdir));
		}
	}

	filesystem::sanitize_directory_path(tmp_path);

	// time to save the well-formatted root directory
	_rootdir = tmp_path;

	// time to build internal data structures
	_handlers.resize(f.size());
	_dicts.resize(f.size());
	_invalid_columns.resize(f.size(), false);
	_invalid_sel_handlers.resize(f.size());

	try {
		for (size_t i = 0; i < f.size(); ++i) {
			std::string column_path =
			    tmp_path + std::to_string(i) + description::data_file_extension;

			if ((column_path.size()) >= PATH_MAX) {
				throw std::length_error("column file path too long");
			}

			_handlers[i] = std::make_unique<file_write_handler>(column_path, f[i]);

			std::string invalid_sel_column_path =
			    _rootdir + std::to_string(i) + pvcop::db::description::invalid_sel_file_extension;
			if ((invalid_sel_column_path.size()) >= PATH_MAX) {
				throw std::length_error("column file path too long");
			}
			_invalid_sel_handlers[i] = std::make_unique<sel_write_handler>(invalid_sel_column_path);

			if (f[i] == "string") {
				_dicts[i] = std::make_unique<write_dict>();
			}
		}
	} catch (std::exception& e) {
		throw exception::collector_error(std::string("Fail to initialize: ") + e.what());
	}

	_column_count = _handlers.size();
}

/*****************************************************************************
 * pvcop::db::collector::~collector
 *****************************************************************************/

pvcop::db::collector::~collector()
{
	/*assert(filesystem::check_access(_rootdir,
	                                filesystem::access_mode::READ,
	                                filesystem::entry_type::FILE));*/
}

/*****************************************************************************
 * pvcop::db::collector::close
 *****************************************************************************/

void pvcop::db::collector::close()
{
	description::get_column_info_f get = [&](size_t i) {
		if (not this->_handlers[i]) {
			throw pvcop::db::exception::invalid_column("No valid handler for column : " +
			                                           std::to_string(i));
		}
		return this->_handlers[i]->type();
	};

	description::save(_rootdir, _row_count, _column_count, get);

	for (size_t i = 0; i < _dicts.size(); ++i) {
		const auto dict = _dicts[i].get();

		if (dict != nullptr) {
			try {
				std::string dict_path =
				    _rootdir + std::to_string(i) + description::dict_file_extension;
				dict->save(dict_path);
			} catch (std::ios::failure& e) {
				throw std::ios::failure(std::string("can not save dictionary of column ") +
				                        std::to_string(i) + ": " + e.what());
			}
		}
	}

	clean_handlers();
	clean_dicts();
}

/*****************************************************************************
 * pvcop::db::collector::operator bool
 *****************************************************************************/

pvcop::db::collector::operator bool() const
{
	return _handlers.size() != 0;
}

/*****************************************************************************
 * pvcop::db::collector::type
 *****************************************************************************/

pvcop::db::type_t pvcop::db::collector::type(size_t index) const
{
	if (index >= _dicts.size()) {
		throw pvcop::db::exception::invalid_column("Out of range column index : " +
		                                           std::to_string(index));
	}

	return _handlers[index]->type();
}

/*****************************************************************************
 * pvcop::db::collector::dict
 *****************************************************************************/

pvcop::db::write_dict* pvcop::db::collector::dict(size_t index) const
{
	if (index >= _dicts.size()) {
		throw pvcop::db::exception::invalid_column("Out of range column index : " +
		                                           std::to_string(index));
	}

	return _dicts[index].get();
}

/*****************************************************************************
 * pvcop::db::collector::set_dict
 *****************************************************************************/

void pvcop::db::collector::set_dict(size_t index, std::unique_ptr<write_dict> dict)
{
	if (index >= _dicts.size()) {
		throw pvcop::db::exception::invalid_column("Out of range column index : " +
		                                           std::to_string(index));
	}

	_dicts[index] = std::move(dict);
}

/*****************************************************************************
 * pvcop::db::collector::clean_handlers
 *****************************************************************************/

void pvcop::db::collector::clean_handlers()
{
	_handlers.clear();
	_invalid_sel_handlers.clear();
	_invalid_columns.clear();
}

/*****************************************************************************
 * pvcop::db::collector::clean_dicts
 *****************************************************************************/

void pvcop::db::collector::clean_dicts()
{
	_dicts.clear();
}
