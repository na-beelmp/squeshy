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

#include <pvcop/db/sink.h>

#include <pvcop/core/mempage.h>

#include <pvcop/db/collector.h>

#include <pvcop/db/file_write_handler.h>

#include <pvcop/types/factory.h>

#include <cstring>
#include <limits>

/**
 * @class pvcop::db::sink
 *
 * @todo must reentrant
 *
 * @note it could be smart to convert a file_page into a core::array. But how
 * having the right type? Can not, user's problem
 *
 * @note does a sink must register itself to the collector
 *
 * @warning
 * - not formalized
 */

/*****************************************************************************
 * pvcop::db::sink
 *****************************************************************************/

/**
 * @note
 * - must be registered to its collector
 */
pvcop::db::sink::sink(collector& c)
    : _collector(c), _string_formatter(pvcop::types::factory::create("string", ""))
{
	if (c) {
		_pages.resize(c.column_count());
		_invalid_sel_pages.resize(c.column_count());
	}
}

/*****************************************************************************
 * pvcop::db::~sink
 *****************************************************************************/

pvcop::db::sink::~sink()
{
	for (size_t i = 0; i < _pages.size(); ++i) {
		auto& p = page(i);
		auto& handler = _collector._handlers[i];

		handler->release_page(p);
		p = core::mempage();
	}

	// ensure invalid selection files are completely written on disk
	const size_t row_count = _collector.get_row_counter();
	for (size_t col = 0; col < _collector._invalid_sel_handlers.size(); col++) {
		if (_collector._invalid_columns[col]) {
			_collector._invalid_sel_handlers[col]->ensure_allocated(row_count);
		}
	}
}


/*****************************************************************************
 * pvcop::db::sink::type
 *****************************************************************************/

pvcop::db::type_t pvcop::db::sink::type(size_t index) const
{
	return _collector.type(index);
}
/*****************************************************************************
 * pvcop::db::sink::dict
 *****************************************************************************/

pvcop::db::write_dict* pvcop::db::sink::dict(size_t index) const
{
	return _collector.dict(index);
}

/*****************************************************************************
 * pvcop::db::sink::request_page
 *****************************************************************************/

bool pvcop::db::sink::request_page(const size_t col, const size_t row)
{
	assert(col < _pages.size());
	return _collector._handlers[col]->request_page(row, _pages[col]);
}

/*****************************************************************************
 * pvcop::db::sink::release_page
 *****************************************************************************/

void pvcop::db::sink::release_page(const size_t col)
{
	assert(col < _pages.size());
	auto& page = _pages[col];
	if (page) {
		_collector._handlers[col]->release_page(page);
		page = core::mempage();
	}
}

/*****************************************************************************
 * pvcop::db::sink::check_fault
 *****************************************************************************/

size_t pvcop::db::sink::check_fault(const size_t col, const size_t row) const
{
	return _pages[col].check_fault(row);
}

/*****************************************************************************
 * pvcop::db::sink::check_fault
 *****************************************************************************/

size_t pvcop::db::sink::check_fault(const size_t row) const
{
	size_t value = std::numeric_limits<size_t>::max();

	for (const auto& page : _pages) {
		size_t v = page.check_fault(row);
		if (v == 0) {
			return 0;
		}
		value = std::min(value, v);
	}

	return value;
}

/*****************************************************************************
 * pvcop::db::sink::refresh_pages
 *****************************************************************************/

bool pvcop::db::sink::refresh_pages(const size_t row)
{
	bool ret = true;
	for (size_t i = 0; i < _pages.size() and ret; ++i) {
		ret &= refresh_page(i, row);
	}
	return ret;
}

bool pvcop::db::sink::refresh_page(const size_t col, const size_t row)
{
	auto& page = _pages[col];

	if (page.check(row)) {
		return true;
	}

	auto& handler = _collector._handlers[col];

	handler->release_page(page);

	return handler->request_page(row, page);
}

/*****************************************************************************
 * pvcop::db::sink::increment_row_count
 *****************************************************************************/

void pvcop::db::sink::increment_row_count(const size_t count)
{
	_collector.get_row_counter() += count;
}

void pvcop::db::sink::write_chunk_by_column(const size_t begin_row,
                                            const size_t row_count,
                                            const sink::columns_chunk_t& columns)
{
	const size_t column_count = _collector.column_count();

	assert(columns.size() == _collector.column_count());

	for (size_t col = 0; col < column_count; col++) {
		size_t local_row_index = 0;
		sink::column_chunk_t const& column_chunk = columns[col];
		size_t remain = row_count;

		while (remain) {
			size_t pcount = check_fault(col, begin_row + local_row_index);
			if (pcount == 0) {
				bool ret = refresh_page(col, begin_row + local_row_index);
				(void) ret;
				assert(ret == true);
				pcount = check_fault(col, begin_row + local_row_index);
				assert(pcount != 0);
			}
			pcount = std::min(pcount, remain);

			std::memcpy(
				((uint8_t*)page(col).base()) + page(col).relative_index(begin_row + local_row_index) * column_chunk.row_bytes_count,
				((uint8_t*)column_chunk.buffer) + local_row_index * column_chunk.row_bytes_count,
				pcount * column_chunk.row_bytes_count
			);

			local_row_index += pcount;
			remain -= pcount;
		}
	}

	increment_row_count(row_count);
}

void pvcop::db::sink::set_column_dict(size_t column_index, std::unique_ptr<db::write_dict> dict)
{
	_collector.set_dict(column_index, std::move(dict));
}

void pvcop::db::sink::set_chunk_null_bitmap(size_t col, size_t begin_row, size_t row_count, const uint8_t* chunk_null_bitmap)
{
	constexpr const int digits = std::numeric_limits<uint8_t>::digits;
	assert(begin_row % digits == 0);

	core::mempage& page = _invalid_sel_pages[col];
	size_t local_row_index = 0;
	size_t remain = row_count;
	while (remain) {
		size_t pcount = page.check_fault(begin_row + local_row_index);
		if (pcount == 0) {
			_collector._invalid_sel_handlers[col]->release_page(page);
			bool ret = _collector._invalid_sel_handlers[col]->request_page(begin_row + local_row_index, page);
			(void) ret;
			assert(ret == true);
			pcount = page.check_fault(begin_row + local_row_index);
			assert(pcount != 0);
		}
		pcount = std::min(pcount, remain);

		size_t dst_offset = page.relative_index(begin_row + local_row_index)  / digits;
		size_t src_offset = local_row_index / digits;
		const uint8_t* src = chunk_null_bitmap + src_offset;
		uint8_t* dst = ((uint8_t*)page.base()) + dst_offset;
		size_t size = (pcount + digits - 1) / digits;

		std::memcpy(dst, src, size);

		local_row_index += pcount;
		remain -= pcount;
	}

	_collector._invalid_columns[col] = true;
}