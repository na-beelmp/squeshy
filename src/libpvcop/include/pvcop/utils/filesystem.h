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

#ifndef __PVCOP_FILESYSTEM_H__
#define __PVCOP_FILESYSTEM_H__

#include <string>
#include <fstream>

namespace pvcop
{

namespace filesystem
{

enum class access_mode { READ, WRITE };

enum class entry_type { FILE, DIR };

bool check_access(const std::string& path, access_mode m, entry_type t);

/**
 * Checks and create recursively a path
 *
 * @param path the path to create
 *
 * @return 0 when successful, an error code otherwise, errno may be set
 */
int make_path(const std::string& path);

/**
 * Make sure a path is well-formed.
 *
 * @param path the directory path to sanitize
 */
void sanitize_directory_path(std::string& path);

/**
 * Check if two files have the same content
 *
 * @param filename1 path of the first file
 * @param filename2 path of the second file
 *
 * @return true if both files have the same content
 *         false otherwise
 */
bool compare_files(const char* filename1, const char* filename2);

} // namespace pvcop::filesystem

} // namespace pvcop

#endif // __PVCOP_FILESYSTEM_H__
