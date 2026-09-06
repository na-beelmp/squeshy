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

#ifndef __IMPORT_PIPELINE__
#define __IMPORT_PIPELINE__

#include <pvcop/collector.h>
#include <pvcop/sink.h>

#include <fcntl.h>
#include <math.h>
#include <stdlib.h> // posix_memalign
#ifdef _WIN32
#include "mmap-windows.c"
#include <windows.h>
#else
#include <sys/mman.h>
#include <unistd.h>
#endif
#include <sys/stat.h>

#include <pvlogger.h>
#include <pvhwloc.h>
#include <csvreader/csvreader.h>

#include <tbb/parallel_pipeline.h>
#include <tbb/scalable_allocator.h>

#include <pvcop/sink.h>

#include <algorithm>
#include <atomic>
#include <fstream>
#include <libgen.h>

namespace import_pipeline
{

#define USE_MMAP 0

class chunk
{
  public:
	chunk()
	    : _begin_row(0)
	    , _row_count(0)
	    , _remaining_char_count(0)
	    , _buffer(nullptr)
	    , _buffer_size(0)
	    , _physical_buffer(nullptr)
	    , _physical_buffer_size(0)
	{
	}

	chunk(size_t index,
	      size_t total_buffer_size,
	      size_t begin_row,
	      size_t previous_chunk_remaining_char_count,
	      int file_handler)
	    : chunk()
	{
#ifdef _WIN32
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		static const size_t page_size = si.dwPageSize;
#else
		static const size_t page_size = sysconf(_SC_PAGE_SIZE);
#endif

		_begin_row = begin_row;

		size_t start_offset = index * MAX_BUFFER_SIZE;
		size_t end_offset = (index + 1) * MAX_BUFFER_SIZE;

		_buffer_size =
		    end_offset > total_buffer_size ? total_buffer_size - start_offset : MAX_BUFFER_SIZE;
		_buffer_size += previous_chunk_remaining_char_count;

		size_t effective_start_offset = start_offset - previous_chunk_remaining_char_count;
		size_t aligned_start_offset = effective_start_offset & -page_size;

		_physical_buffer_size = _buffer_size + (effective_start_offset - aligned_start_offset);

#if USE_MMAP
		int mmap_flags = MAP_PRIVATE | MAP_POPULATE | MAP_NORESERVE;
#ifndef _WIN32
		mmap_flags |= MAP_NORESERVE;
#endif
		if ((_physical_buffer = (char*)mmap(NULL, _physical_buffer_size, PROT_WRITE, mmap_flags,
		                                    file_handler, aligned_start_offset)) == MAP_FAILED) {
			pvlogger::error() << "mmap failed for chunk #" << index << std::endl;
		}
#else
		// Try to set O_DIRECT flag (lack of optimization if failure)
#if __linux__
		fcntl(file_handler, F_SETFL, fcntl(file_handler, F_GETFL, 0) | O_DIRECT);
#endif

		errno = 0;
		if (lseek(file_handler, aligned_start_offset, SEEK_SET) >= 0) {

			if (scalable_posix_memalign((void**)&_physical_buffer, page_size,
			                            _physical_buffer_size) == 0) {

				if ((_physical_buffer_size % page_size) != 0) {
					// remove O_DIRECT flag to read the end of the file
#if __linux__
					fcntl(file_handler, F_SETFL, fcntl(file_handler, F_GETFL, 0) & ~O_DIRECT);
#endif
				}

				errno = 0;
				int64_t read_bytes = read(file_handler, _physical_buffer, _physical_buffer_size);
				if (read_bytes != (int64_t)_physical_buffer_size) {
					pvlogger::error() << "Read failed for chunk #" << index << " ("
					                  << strerror(errno) << ")" << std::endl;
				}
			} else {
				pvlogger::error() << "Allocation failed for chunk #" << index << std::endl;
			}
		} else {
			pvlogger::error() << "Seek failed for chunk #" << index << " (" << strerror(errno)
			                  << ")" << std::endl;
		}
#endif
		_buffer = _physical_buffer + (effective_start_offset - aligned_start_offset);

		count_rows();

		_buffer_size -= _remaining_char_count;
	}

	~chunk()
	{
#if USE_MMAP
		if (munmap(_physical_buffer, _physical_buffer_size) != 0) {
			pvlogger::error() << "munmap failed" << std::endl;
		}
#else
		scalable_free(_physical_buffer);
#endif
	}

	inline size_t begin_row() const { return _begin_row; }
	inline size_t row_count() const { return _row_count; }
	inline size_t remaining_char_count() const { return _remaining_char_count; }

	inline char* buffer() const { return _buffer; }
	inline size_t buffer_size() const { return _buffer_size; }

  private:
	void count_rows()
	{
		char* ptr = _buffer;
		char* last_ptr = ptr;

		while ((ptr = (char*)memchr(ptr, '\n', (_buffer + _buffer_size - ptr)))) {
			last_ptr = ++ptr;
			++_row_count;
		}

		_remaining_char_count = (_buffer + _buffer_size) - last_ptr;
	}

  public:
	static constexpr const size_t MAX_BUFFER_SIZE = 2 * 1024 * 1024;

  private:
	size_t _begin_row;
	size_t _row_count;
	size_t _remaining_char_count;

	char* _buffer;
	size_t _buffer_size;

	char* _physical_buffer;
	size_t _physical_buffer_size;
};

using chunk_parsing_function = std::function<void(const chunk& chunk)>;

size_t import_file(const char* file_path, const chunk_parsing_function& f)
{
	size_t file_size = 0;
	struct stat file_stat;
	int file_handler;

	if (stat(file_path, &file_stat) == 0) {
		file_size = file_stat.st_size;
		file_handler = open(file_path, O_RDONLY);
#if __linux__
		posix_fadvise(file_handler, 0, 0,
		              POSIX_FADV_SEQUENTIAL | POSIX_FADV_NOREUSE | POSIX_FADV_DONTNEED);
#endif
	} else {
		pvlogger::error() << "Unable to open " << file_path << std::endl;
		return 0;
	}

	size_t chunk_count = ceil((double)file_size / chunk::MAX_BUFFER_SIZE);
	size_t chunk_index = 0;

	size_t total_row_count = 0;
	size_t previous_chunk_remaining_char_count = 0;

	tbb::parallel_pipeline(
	    /*max_number_of_live_token=*/pvhwloc::core_count() * 2,
	    tbb::make_filter<void, chunk*>(
	        tbb::filter_mode::serial_in_order,
	        [&](tbb::flow_control& fc) -> chunk* {
		        if (chunk_index < chunk_count) {

			        chunk* chunk = tbb::scalable_allocator<import_pipeline::chunk>().allocate(
			            sizeof(import_pipeline::chunk));
			        new (chunk)
			            import_pipeline::chunk(chunk_index++, file_size, total_row_count,
			                                   previous_chunk_remaining_char_count, file_handler);

			        previous_chunk_remaining_char_count = chunk->remaining_char_count();
			        total_row_count += chunk->row_count();

			        return chunk;
		        } else {
			        fc.stop();
			        return nullptr;
		        }
		    }) &
	        tbb::make_filter<chunk*, void>(tbb::filter_mode::parallel, [&](chunk* chunk) {
		        f(*chunk);
		        chunk->~chunk();
		        tbb::scalable_allocator<import_pipeline::chunk>().deallocate(
		            chunk, sizeof(import_pipeline::chunk));
		    }));

	close(file_handler);

	return total_row_count;
}

static pvcop::sink::field_t* parse_csv(const import_pipeline::chunk& chunk,
                                       const size_t column_count)
{
	pvcop::sink::field_t* fields = new pvcop::sink::field_t[chunk.row_count() * column_count];

	std::vector<csv::reader::entry_info_t> entries(column_count);
	csv::reader reader(chunk.buffer(), chunk.buffer_size(), column_count, ',', '"', true);

	for (size_t row = 0; row < chunk.row_count(); row++) {

		char* row_buffer = nullptr;
		size_t row_buffer_size;

		row_buffer = reader.read_next_line(&row_buffer_size);

		if (row_buffer_size != 0) {

			if (reader.process_line(entries.data(), row_buffer, row_buffer_size) != false) {

				for (size_t col = 0; col < column_count; ++col) {
#ifdef CHUNK_BY_COLUMN
					new (fields + col * chunk.row_count() + row)
#else
					new (fields + row * column_count + col)
#endif
					    pvcop::sink::field_t(row_buffer + entries[col].pos, entries[col].len);
				}
			}
		}
	}

	return fields;
}

} // namespace import_pipeline

namespace pvcop
{
size_t import(const char* csv_file_path,
              const formatter_desc_list& formatter_descs,
              const char* collector_path)
{
	std::atomic<size_t> failing_import(0);

	collector collector(collector_path, formatter_descs);

	const size_t column_count = collector.column_count();

	import_pipeline::import_file(csv_file_path, [&](const import_pipeline::chunk& chunk) {

		const size_t begin_row = chunk.begin_row();

		const std::unique_ptr<sink::field_t[]> fields(parse_csv(chunk, column_count));

		pvcop::sink sink(collector);

#ifdef CHUNK_BY_COLUMN
		sink.write_chunk_by_column(begin_row, chunk.row_count(), fields.get());
#else
		sink.write_chunk_by_row(begin_row, chunk.row_count(), fields.get());
#endif
	});

#ifndef _WIN32
	sync();
#endif
	collector.close();

	return failing_import;
}
}

#endif // __IMPORT_PIPELINE__
