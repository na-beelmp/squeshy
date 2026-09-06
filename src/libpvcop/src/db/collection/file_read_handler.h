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

#ifndef __PVCOP_DB_FILE_READ_HANDLER_H__
#define __PVCOP_DB_FILE_READ_HANDLER_H__

#include <pvcop/db/file_handler.h>

#include <mutex>

namespace pvcop
{

namespace db
{

/**
 * Handler of read-only memory mapped file.
 *
 * This class manages accesses to a file content by mapping/unmapping it
 * in/from memory. To avoid wasting memory with mapped files, it implements a
 * resource request/release logic. Reference counting is used to track columns
 * usage and unmap them when they become useless. column are also mapped the
 * first time they are requested.
 *
 * As Linux really invalidates unmapped pages when there is not enough free
 * space for another mapping/allocation, mapping again a file will simply
 * reuse pages which are still present in the disk-cache.
 *
 * As pvcop is designed to be used in multi-threaded program, the methods
 * request() and release() have been made thread-safe by using a mutex.
 *
 * @warning There is no request/release tracking, be sure to have coherent
 * calls to those two methods.
 *
 * @warning The file can only be entirely mapped, it is also not possible to
 * map a subset of the file.
 *
 * @note is partial mapping useful in case of really huge file?
 *
 * @note how to distribute the mapping on the NUMA memory nodes?
 */
class file_read_handler : public file_handler
{
  public:
	/**
	 * Constructor
	 *
	 * @param filename the full pathname to the file to map
	 * @param t the type of the data
	 * @param page_size size of the page
	 * @param count the number of values
	 */
	file_read_handler(const std::string& filepath,
	                  type_t t,
	                  size_t count,
	                  size_t page_size = small_page_size);

	/**
	 * Destructor
	 */
	~file_read_handler();

  public:
	/**
	 * Get the memory size
	 *
	 * @return the memory size in bytes
	 */
	size_t size() const { return _mem_size; }

  public:
	/**
	 * Request the memory base address of this mapped file
	 *
	 * @return the memory base address
	 *
	 * @note this method is thread-safe
	 */
	void* request();

	/**
	 * Release the memory base address of this mapped file
	 *
	 * @note this method is thread-safe
	 */
	void release();

  private:
	std::mutex _mutex;
	void* _base_address;
	const size_t _value_count;
	size_t _ref_count;
	size_t _mem_size;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_FILE_READ_HANDLER_H__
