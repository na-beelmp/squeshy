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

#include <pvcop/db/file_write_handler.h>

#include <pvcop/core/mempage.h>
#include <pvcop/db/exceptions/pagination_error.h>

#include <cstring>
#include <cerrno>

#include <sys/stat.h>
#ifdef _WIN32
#include "mmap-windows.c"
#else
#include <sys/mman.h>
#endif
#include <fcntl.h>

/**
 * @class file_write_handler
 *
 * @todo can binding pages to nodes be relevant?
 *
 * @todo can prefaulting the pages be relevant?
 *
 * @todo who release pages? sink must be deleted before its collector
 */

/*****************************************************************************
 * pvcop::db::file_write_handler::file_write_handler
 *****************************************************************************/

pvcop::db::file_write_handler::file_write_handler(const std::string& filepath,
                                                  type_t t,
                                                  size_t page_size /* = huge_page_size */)
    : file_handler(filepath, t, page_size, WRITE)
{
}

/*****************************************************************************
 * pvcop::db::file_write_handler::~file_write_handler
 *****************************************************************************/

pvcop::db::file_write_handler::~file_write_handler()
= default;

/*****************************************************************************
 * pvcop::db::file_write_handler::request_page
 *****************************************************************************/

bool pvcop::db::file_write_handler::request_page(size_t index, pvcop::core::mempage& page) const
{
	const size_t page_count = index / _element_per_page;
	const size_t mem_base_offset = page_count * _page_size;
	const size_t element_base_offset = page_count * _element_per_page;

	if (posix_fallocate(fd(), mem_base_offset, _page_size) != 0) {
		return false;
	}

	int flags = MAP_SHARED;
#ifndef _WIN32
		flags |= MAP_NORESERVE;
#endif
	void* addr =
	    mmap(nullptr, _page_size, PROT_WRITE, flags, fd(), mem_base_offset);

	if (addr == MAP_FAILED) {
		std::ostringstream error;
		error << "MAP_FAILED: " << std::strerror(errno);
		throw db::exception::pagination_error(error.str());
	}

	page = core::mempage(addr, element_base_offset, _element_per_page);

	return true;
}

/*****************************************************************************
 * pvcop::db::file_write_handler::release_page
 *****************************************************************************/

void pvcop::db::file_write_handler::release_page(pvcop::core::mempage& page) const
{
	if (page) {
		munmap(page.base(), _page_size);
	}
}
