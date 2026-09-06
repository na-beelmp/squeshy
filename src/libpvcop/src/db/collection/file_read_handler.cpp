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

#include <db/collection/file_read_handler.h>
#include <pvcop/db/exceptions/invalid_read_handler.h>
#include <pvcop/db/types_manager.h>

#include <cerrno>
#include <filesystem>

#include <sys/stat.h>
#ifdef _WIN32
#include "mmap-windows.c"
#else
#include <sys/mman.h>
#endif

/*****************************************************************************
 * pvcop::db::file_read_handler::file_read_handler
 *****************************************************************************/

pvcop::db::file_read_handler::file_read_handler(const std::string& filepath,
                                                type_t t,
                                                size_t count,
                                                size_t page_size /* = small_page_size */)
    : file_handler(filepath, t, page_size, READ)
    , _base_address(nullptr)
    , _value_count(count)
    , _ref_count(0)
    , _mem_size(0)
{
}

/*****************************************************************************
 * pvcop::db::file_read_handler::~file_read_handler
 *****************************************************************************/

pvcop::db::file_read_handler::~file_read_handler()
= default;

/*****************************************************************************
 * pvcop::db::file_read_handler::request
 *****************************************************************************/

void* pvcop::db::file_read_handler::request()
{
	/* the two first test do not need to be protected by the lock guard
	 * because _fd and _value_count are constants during the instance
	 * whole life.
	 */
	if (fd() == -1) {
		throw pvcop::db::exception::invalid_read_handler(
		    "Invalid file descriptor for read handler");
	}

	if (_value_count == 0) {
		throw pvcop::db::exception::invalid_read_handler("Invalid count for read handler");
	}

	std::lock_guard<std::mutex> lg(_mutex);

	if (_base_address == nullptr) {
		/* first request for this file
		 */
		errno = 0;

#ifdef _WIN32
		struct _stat file_stat;
		std::wstring wpath = std::filesystem::path(_filepath).wstring();
		if (_wstat(wpath.c_str(), &file_stat) != 0) {
#else
		struct stat file_stat;
		if (stat(_filepath.c_str(), &file_stat) != 0) {
#endif
			throw pvcop::db::exception::invalid_read_handler(
			    "File doesn't exists for read handler");
		}

		size_t file_size = file_stat.st_size;

		if (file_size == 0) {
			throw pvcop::db::exception::invalid_read_handler("File is empty for read handler");
		}

		size_t data_size = types_manager::traits(type()).to_mem_size(_value_count);

		if (data_size > file_size) {
			throw pvcop::db::exception::invalid_read_handler(
			    "not enough elements in file for read handler");
		}

		int flags = MAP_PRIVATE;
#ifndef _WIN32
		flags |= MAP_NORESERVE;
#endif
		void* addr = mmap(nullptr, data_size, PROT_READ, flags, fd(), 0);

		if (addr == MAP_FAILED) {
			throw pvcop::db::exception::invalid_read_handler("File can't be mapped in memory");
		}

		_base_address = addr;
		_mem_size = data_size;
	}

	/* we have a valid memory block address to return, the reference
	 * counter can also be updated
	 */
	++_ref_count;

	return _base_address;
}

/*****************************************************************************
 * pvcop::db::file_read_handler::release
 *****************************************************************************/

void pvcop::db::file_read_handler::release()
{
	/* as in ::request
	 */
	if (fd() == -1) {
		return;
	}

	if (_value_count == 0) {
		return;
	}

	std::lock_guard<std::mutex> lg(_mutex);

	if (_ref_count == 0) {
		// skip at least stupid calls
		return;
	}

	--_ref_count;

	if (_ref_count != 0) {
		return;
	}

	if (_base_address) {
		size_t data_size = types_manager::traits(type()).to_mem_size(_value_count);

		munmap(_base_address, data_size);
		_base_address = nullptr;
		_mem_size = 0;
	}
}
