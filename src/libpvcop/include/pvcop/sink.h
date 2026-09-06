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

#ifndef __PVCOP_TYPES_SINK_H__
#define __PVCOP_TYPES_SINK_H__

#include <memory>

#include <pvcop/db/sink.h>
#include <pvcop/core/mempage.h>
#include <pvcop/core/pagedarray.h>

namespace pvcop
{

class collector;

/**
 * This class is an accessor to write row oriented data in a collector using formatters
 * according to the specified format
 */
class sink : public pvcop::db::sink
{

  public:
	/**
	 * Field_t is a field to save on the HDD.
	 *
	 * @note It has ownership of the buffer as it will be the value save on the HDD,
	 * keeping this value in RAM may lead to de-synchronization between HDD and RAM.
	 */
	struct field_t {
		field_t() : buffer(nullptr), size(0) {}
		field_t(const char* b, size_t s) : buffer(b), size(s) {}

		const char* buffer;
		size_t size;
	};

  public:
	/**
	 * Constructor
	 *
	 * @param collector the target collector
	 */
	explicit sink(collector& collector);

  public:
	/**
	 * Writes a chunk of fields to the sink
	 *
	 * @param begin_row index of the first row
	 * @param row_count number of rows
	 * @param fields array of fields
	 *
	 * @throws pvcop::db::exception::pagination_error
	 * @throws pvcop::types::exception::partially_converted_chunk_error
	 */
	void write_chunk_by_row(const size_t begin_row, const size_t row_count, const field_t* fields);

	/**
	 * Writes a chunk of fields to the sink
	 *
	 * @param begin_row index of the first row
	 * @param row_count number of rows
	 * @param fields array of fields
	 *
	 * @throws pvcop::db::exception::pagination_error
	 * @throws pvcop::types::exception::partially_converted_chunk_error
	 */
	void
	write_chunk_by_column(const size_t begin_row, const size_t row_count, const field_t* fields);

	void set_invalid(size_t col, size_t row);

  private:
	/**
	 * Writes a field at a specific position
	 *
	 * @param field the field to write
	 * @param row the row position to write to
	 * @param col the column position to write to
	 *
	 * @return @c true if the field has been written; @c false otherwise
	 */
	void write_field(const pvcop::sink::field_t& field, const size_t row, const size_t col);

  private:
	collector& _collector;
};

} // namespace pvcop

#endif // __PVCOP_TYPES_SINK_H__
