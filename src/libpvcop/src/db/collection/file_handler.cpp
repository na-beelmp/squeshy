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

#include <pvcop/db/file_handler.h>
#include <pvcop/db/types_manager.h>

#include <fcntl.h>
#include <cstring>

#include <unistd.h>

#include <filesystem>

#ifndef _WIN32
size_t const pvcop::db::file_handler::small_page_size = sysconf(_SC_PAGE_SIZE);
size_t const pvcop::db::file_handler::huge_page_size = 2 * 1024 * 1024;
#else
#include <windows.h>
size_t const pvcop::db::file_handler::small_page_size = GetLargePageMinimum();
size_t const pvcop::db::file_handler::huge_page_size = small_page_size;
#endif

/*****************************************************************************
 * pvcop::db::file_handler::file_handler
 *****************************************************************************/

pvcop::db::file_handler::file_handler(const std::string& filepath,
                                      type_t t,
                                      size_t page_size,
                                      access_mode mode)
    : _filepath(filepath)
    , _type(t)
    , _element_per_page(types_manager::traits(t).from_mem_size(page_size))
    , _page_size(page_size)
{
	mode_t open_mode = 0640;
	int flags = 0;
	if (mode == READ) {
		flags = O_RDONLY;
	} else {
		flags = O_CREAT | O_TRUNC | O_RDWR;
	}
#ifdef _WIN32
	std::wstring wpath = std::filesystem::path(_filepath).wstring();
	_fd = _wopen(wpath.c_str(), flags, open_mode);
	// Disable handles inheritence with subprocesses to prevent Windows file locking problems
	SetHandleInformation((HANDLE)_get_osfhandle(_fd), HANDLE_FLAG_INHERIT, 0);
#else
	// force global namespace to resolve unistd.h/open(...)
	_fd = ::open(_filepath.c_str(), flags, open_mode);
#endif
}

/*****************************************************************************
 * pvcop::db::file_handler::~file_handler
 *****************************************************************************/

pvcop::db::file_handler::~file_handler()
{
	close();
}

/*****************************************************************************
 * pvcop::db::file_handler::remove
 *****************************************************************************/

void pvcop::db::file_handler::remove()
{
	close();
	std::filesystem::remove(_filepath);
}

/*****************************************************************************
 * pvcop::db::file_handler::close
 *****************************************************************************/

void pvcop::db::file_handler::close()
{
	if (_fd != -1) {
		// force global namespace to resolve unistd.h/close(int)
		::close(_fd);
		_fd = -1;
	}
}
