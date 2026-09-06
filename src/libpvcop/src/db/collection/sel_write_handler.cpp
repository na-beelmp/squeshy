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

#include <pvcop/db/sel_write_handler.h>

#include <pvcop/db/types_manager.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <filesystem>

pvcop::db::sel_write_handler::sel_write_handler(const std::string& filepath)
    : db::file_write_handler(filepath, "number_bool", small_page_size)
{
}

pvcop::db::sel_write_handler::~sel_write_handler()
{
	// Remove file if empty
#ifdef _WIN32
	struct _stat st;
	std::wstring wpath = std::filesystem::path(_filepath).wstring();
	_wstat(wpath.c_str(), &st);
#else
	struct stat st;
	stat(_filepath.c_str(), &st);
#endif

	if (st.st_size == 0) {
		remove();
	}
}

bool pvcop::db::sel_write_handler::ensure_allocated(size_t row_count)
{
	const size_t file_size = pvcop::db::types_manager::traits(_type).to_mem_size(row_count);

	return posix_fallocate(fd(), 0, file_size) == 0;
}
