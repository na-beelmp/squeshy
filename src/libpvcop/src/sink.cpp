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


#include <pvcop/sink.h>
#include <pvcop/collector.h>

#include <pvcop/db/file_write_handler.h>

#include <pvcop/types/string.h>
#include <pvcop/types/formatter/formatter_interface.h>

#include <pvcop/db/sel_write_handler.h>
#include <pvcop/core/impl/bit.h>

pvcop::sink::sink(pvcop::collector& c) : pvcop::db::sink(c), _collector(c)
{
}

void pvcop::sink::write_field(const pvcop::sink::field_t& field, const size_t row, const size_t col)
{
	// Unmatching row
	if (field.buffer == nullptr) {
		_collector.formatter(col)->set_default_value(page(col), row);
		return;
	}

	// Convert string to null-terminated string
	std::string str(field.buffer, field.size);

	bool ret = _collector.formatter(col)->from_string(str.c_str(), page(col), row);

	if (not ret) {
		set_invalid(col, row);
	}
}

void pvcop::sink::set_invalid(size_t col, size_t row)
{
	// set selection bit
	core::mempage& page = _invalid_sel_pages[col];
	if (page.check_fault(row) == 0) {
		_collector._invalid_sel_handlers[col]->release_page(page);
		_collector._invalid_sel_handlers[col]->request_page(row, page);
	}
	/* Set the bit atomically rather than through pagedarray<bool>: the selection is a
	 * bit array, so 'p[index] = true' is a read-modify-write of the word covering 64
	 * rows, and the sinks importing different chunks of a same column share that word
	 * -- whoever writes last wipes the bits the others just set. Losing one does not
	 * corrupt the value in place: the row reads back as valid, and the invalid
	 * dictionary index held in it gets formatted as a genuine value, yielding a number
	 * out of nowhere or a date boost refuses to build. A fetch_or keeps this lock-free;
	 * relaxed ordering is enough, as the bits are only read back once every sink is
	 * done and the collector has been closed. */
	namespace bit_manip = core::__impl::bit_manip;
	auto* const chunks = static_cast<bit_manip::value_type*>(page.base());
	const size_t index = page.relative_index(row);
	__atomic_fetch_or(&chunks[bit_manip::chunk_index(index)],
	                  bit_manip::value_type(1) << bit_manip::bit_index(index), __ATOMIC_RELAXED);

	_collector._invalid_columns[col] = true;
}

void pvcop::sink::write_chunk_by_column(const size_t begin_row,
                                        const size_t row_count,
                                        const sink::field_t* fields)
{
	const size_t column_count = _collector.column_count();

	for (size_t col = 0; col < column_count; col++) {
		size_t local_row_index = 0;
		size_t remain = row_count;

		while (remain) {
			size_t pcount = check_fault(col, begin_row + local_row_index);

			if (pcount == 0) {
				release_page(col);
				request_page(col, begin_row + local_row_index);
				pcount = check_fault(col, begin_row + local_row_index);
				assert(pcount != 0);
			}
			pcount = std::min(pcount, remain);

			for (size_t page_row_index = 0; page_row_index < pcount; page_row_index++) {
				size_t row = begin_row + local_row_index + page_row_index;

				const field_t& field = fields[col * row_count + (local_row_index + page_row_index)];
				write_field(field, row, col);
			}

			local_row_index += pcount;
			remain -= pcount;
		}
	}

	increment_row_count(row_count);
}

void pvcop::sink::write_chunk_by_row(const size_t begin_row,
                                     const size_t row_count,
                                     const field_t* fields)
{
	const size_t column_count = _collector.column_count();

	size_t local_row_index = 0;
	size_t remain = row_count;

	while (remain) {
		size_t pcount = check_fault(begin_row + local_row_index);

		if (pcount == 0) {
			refresh_pages(begin_row + local_row_index);
			pcount = check_fault(begin_row + local_row_index);
			assert(pcount != 0);
		}
		pcount = std::min(pcount, remain);

		for (size_t page_row_index = 0; page_row_index < pcount; page_row_index++) {
			size_t row = begin_row + local_row_index + page_row_index;
			for (size_t col = 0; col < column_count; col++) {

				const field_t& field =
				    fields[(local_row_index + page_row_index) * column_count + col];
				write_field(field, row, col);
			}
		}

		local_row_index += pcount;
		remain -= pcount;
	}

	increment_row_count(row_count);
}
