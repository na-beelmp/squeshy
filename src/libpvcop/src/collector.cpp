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

#include <pvcop/collector.h>

#include <pvcop/db/types.h>
#include <pvcop/db/file_write_handler.h>
#include <pvcop/db/types_manager.h>
#include <pvcop/db/exceptions/collector_error.h>
#include <db/collection/description.h>

#include <pvcop/types/formatter/formatter_interface.h>
#include <pvcop/types/exception/unknown_formatter_error.h>
#include <pvcop/types/factory.h>
#include <pvcop/types/string.h>
#include <pvcop/db/sel_write_handler.h>
#include <write_dict.h>

#include <memory>

static pvcop::db::format storage_format(const pvcop::formatter_desc_list& formatter_descs)
{
	pvcop::db::format db_format;

	bool has_error = false;
	std::string exception_msg = "unknown formatters:";

	for (size_t i = 0; i < formatter_descs.size(); ++i) {
		const auto& fd = formatter_descs[i];

		pvcop::db::type_t type = fd.name();

		if (type == "") {
			has_error = true;
			exception_msg += " column " + std::to_string(i) + " (" + fd.name() + ")";
		}

		db_format.push_back(type);
	}

	if (has_error) {
		db_format.clear();
		throw pvcop::types::exception::unknown_formatter_error(exception_msg);
	}

	return db_format;
}

pvcop::collector::collector(const char* rootdir, const formatter_desc_list& formatter_descs)
    : pvcop::db::collector(rootdir, storage_format(formatter_descs))
    , _formatter_descs(formatter_descs)
{
	_invalid_dicts.resize(formatter_descs.size());

	/* no need to check for formatters validity, it has been done in storage_format(...)
	 */
	try {
		for (size_t i = 0; i < formatter_descs.size(); ++i) {
			_invalid_dicts[i] = std::make_unique<pvcop::write_dict>();

			const auto& fd = formatter_descs[i];
			auto fi = types::factory::create(fd.name(), fd.parameters());

			if (fi->name() == "string") {
				static_cast<types::formatter_string*>(fi)->set_write_dict(dict(i));
			}
			fi->set_invalid_write_dict(_invalid_dicts[i].get());
			_formatters.emplace_back(fi);
		}
	} catch (std::exception& e) {
		throw db::exception::collector_error(std::string("Fail to initialize: ") + e.what());
	}
}

pvcop::collector::~collector()
= default;

void pvcop::collector::close()
{
	_formatter_descs.save(db::collector::rootdir() + formatter_desc_list::filename);

	for (size_t i = 0; i < _invalid_dicts.size(); ++i) {
		const auto dict = _invalid_dicts[i].get();

		try {
			std::string dict_path =
			    _rootdir + std::to_string(i) + pvcop::db::description::invalid_dict_file_extension;
			dict->save(dict_path);
		} catch (std::ios::failure& e) {
			throw std::ios::failure(std::string("can not save dictionary of column ") +
			                        std::to_string(i) + ": " + e.what());
		}
	}

	_invalid_dicts.clear();
	_formatters.clear();

	db::collector::close();
}
