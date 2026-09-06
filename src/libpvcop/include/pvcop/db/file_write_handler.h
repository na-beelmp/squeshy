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

#ifndef __PVCOP_DB_FILE_WRITE_HANDLER_H__
#define __PVCOP_DB_FILE_WRITE_HANDLER_H__

#include <pvcop/db/file_handler.h>

#include <pvcop/core/mempage.h>

#ifndef __linux__
#include "posix_fallocate.h"
#endif

namespace pvcop
{

namespace db
{

/**
 * Handler of mapped file in write mode
 */
class file_write_handler : public file_handler
{
  public:
	/**
	 * Default constructor
	 *
	 * @param filename the full pathname to the file to map
	 * @param t the type of the data
	 * @param epp the number of element fitting in a page
	 */
	file_write_handler(const std::string& filepath, type_t t, size_t page_size = huge_page_size);

	/**
	 * Destructor
	 */
	~file_write_handler();

  public:
	/**
	 * Requests for an page which permits to access to the element at @b index
	 *
	 * @param index the global index in the wanted page
	 * @param page a reference on the resulting page
	 *
	 * @return true if the page has been allocated; false otherwise
	 *
	 * @note when false is returned, the state of page is unchanged
	 *
	 * @note this method is thread-safe only under Linux !
	 */
	bool request_page(size_t index, core::mempage& page) const;

	/**
	 * Releases a page
	 *
	 * @param page a reference on the page to release
	 *
	 * @note this method is thread-safe
	 */
	void release_page(core::mempage& page) const;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_FILE_WRITE_HANDLER_H__
