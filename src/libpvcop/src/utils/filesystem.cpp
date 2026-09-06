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

#include <pvcop/utils/filesystem.h>

#include <algorithm>
#include <iterator>
#include <fstream>
#include <tuple>

#include <cerrno>
#include <climits>

#include <sys/stat.h>

#include <filesystem>

/*****************************************************************************
 * pvcop::filesystem::check_access
 *****************************************************************************/

bool pvcop::filesystem::check_access(const std::string& path,
                                     pvcop::filesystem::access_mode m,
                                     pvcop::filesystem::entry_type t)
{
#ifdef _WIN32
	struct _stat dir_stat;
	std::wstring wpath = std::filesystem::path(path).wstring();
	if (_wstat(wpath.c_str(), &dir_stat) != 0) {
		return false;
	}
#else
	struct stat dir_stat;
	if (stat(path.c_str(), &dir_stat) != 0) {
		return false;
	}
#endif

	const int amode = (m == access_mode::READ) ? (S_IRUSR | S_IRGRP) : (S_IWUSR | S_IWGRP);
	const int atype = (t == entry_type::FILE) ? S_IFREG : S_IFDIR;

	if ((dir_stat.st_mode & atype) == 0) {
		return false;
	}
	if ((dir_stat.st_mode & amode) == 0) {
		return false;
	}

	return true;
}

/*****************************************************************************
 * pvcop::filesystem::make_path
 *****************************************************************************/

int pvcop::filesystem::make_path(const std::string& path)
{
	return std::filesystem::create_directories(std::filesystem::path(path));
}

/*****************************************************************************
 * pvcop::filesystem::sanitize_directory_path
 *****************************************************************************/

void pvcop::filesystem::sanitize_directory_path(std::string& path)
{
	// if there is no trailing '/', we add one
	if (path.back() != '/') {
		path.append("/");
	}
}

/*****************************************************************************
 * pvcop::filesystem::compare_files
 *****************************************************************************/

bool pvcop::filesystem::compare_files(const char* filename1, const char* filename2)
{
	std::ifstream file1{std::filesystem::path(filename1)};
	std::ifstream file2{std::filesystem::path(filename2)};

	file1.seekg(0, std::ios_base::end);
	file2.seekg(0, std::ios_base::end);

	if (file1.tellg() != file2.tellg()) {
		return false;
	}

	std::istreambuf_iterator<char> begin1(file1);
	std::istreambuf_iterator<char> begin2(file2);

	std::istreambuf_iterator<char> end;

	std::tie(begin1, begin2) = std::mismatch(begin1, end, begin2);
	return begin1 == end and begin2 == end;
}
