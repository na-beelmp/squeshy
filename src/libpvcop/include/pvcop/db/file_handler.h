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

#ifndef __PVCOP_DB_FILE_HANDLER_H__
#define __PVCOP_DB_FILE_HANDLER_H__

#include <pvcop/core/mempage.h>
#include <pvcop/db/types.h>

#include <string>

#include <cstddef>

namespace pvcop
{

namespace db
{

/**
 * Abstraction over file and mmap'ed page
 *
 * do pages must have the same size or can their size depend on their word size?
 * I.e. 2Mio for char pages, 4 Mio for short pages, etc.
 *
 * It could help doing only one "big" check for pages, instead of interleaved ones,
 * depending on the memory pressure.
 *
 * is using a mmap'ed page for each sink is better than a page cache?
 *
 * @note this class is not copyable
 */
class file_handler
{
  public:
	using id_t = size_t;

	enum access_mode { READ, WRITE };

  public:
	static const size_t small_page_size;
	static const size_t huge_page_size;

  public:
	/**
	 * Default constructor
	 *
	 * @param filename the full pathname to the file to map
	 * @param t the type of the data
	 * @param page_size size of the page
	 * @param m the access mode
	 */
	file_handler(const std::string& filepath, type_t t, size_t page_size, access_mode mode);

	/**
	 * No copy constructor
	 */
	file_handler(const file_handler&) = delete;

	/**
	 * No move constructor
	 */
	file_handler(file_handler&&) = delete;

	/**
	 * Destructor
	 */
	~file_handler();

  public:
	/**
	 * No copy assignment
	 */
	file_handler& operator=(const file_handler&) = delete;

	/**
	 * No move assignment
	 */
	file_handler& operator=(const file_handler&&) = delete;

  public:
	/**
	 * Get the data type of the file handler
	 *
	 * @return the data type
	 */
	type_t type() const { return _type; }

	/**
	 * Get the element count store in a page given the file type
	 *
	 * @return the number of element which can be stored in a file_page
	 */
	size_t element_per_page() const { return _element_per_page; }

  public:
	/**
	 * Converts to bool to check validity
	 *
	 * @return @c true if the file handler is valid; @c false otherwise
	 */
	operator bool() const { return _fd != -1; }

  protected:
	/**
	 * Removes permanently the corresponding file
	 *
	 * @warning the corresponding file is DELETED!!!
	 */
	void remove();

	/**
	 * Get the file descriptor
	 *
	 * @return the file descriptor
	 */
	int fd() const { return _fd; }

  private:
	/**
	 * Close the data file
	 *
	 * @note must thepages be tracked to be unmmaped/freed or not?
	 */
	void close();

  protected:
	std::string _filepath;
	type_t _type;
	int _fd = -1;
	size_t _element_per_page;
	size_t _page_size;
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_FILE_HANDLER_H__
