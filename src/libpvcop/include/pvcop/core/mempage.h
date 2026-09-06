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

#ifndef __PVCOP_CORE_MEMPAGE_H__
#define __PVCOP_CORE_MEMPAGE_H__

#include <cstddef>

namespace pvcop
{

namespace core
{

/**
 * This class is an abstraction of type agnostic memory page.
 *
 * @todo elaborate on its documentation
 */
class mempage
{
  public:
  public:
	/**
	 * Default constructor
	 */
	mempage() : mempage(nullptr, 0, 0) {}

	/**
	 * Constructor using page information
	 *
	 * @param address the base address of a memory block
	 * @param begin_index the index of the first element in the page
	 * @param size the element number fitting in the page
	 */
	mempage(void* address, size_t begin_index, size_t size)
	    : _base_address(address)
	    , _begin_index(begin_index)
	    , _end_index(begin_index + size)
	    , _size(size)
	{
	}

  public:
	/**
	 * Gets the page base address
	 *
	 * @return the base address of the page
	 */
	void* base() const { return _base_address; }

	/**
	 * Gets the element size fitting in the page
	 *
	 * @return the element size of the page
	 */
	size_t size() const { return _size; }

  public:
	/**
	 * Converts a global index into an index relative to the page
	 *
	 * @param index the global index
	 *
	 * @return the index relative to the page
	 *
	 * @warning this method does not do any range checking; see contain or next_fault_in
	 */
	size_t relative_index(size_t index) const { return index - _begin_index; }

  public:
	/**
	 * Checks if an index matchs the page or not
	 *
	 * @return true if index fits in the page range; false otherwise
	 */
	bool check(size_t index) const { return (index >= _begin_index) && (index < _end_index); }

	/**
	 * Checks if an index matchs the page or tell, how many element can be
	 * written before needing to request a new page.
	 *
	 * @param index the global index to check against
	 *
	 * @return 0 if index does not match the page; the remaining number of
	 * element places otherwise
	 */
	size_t check_fault(size_t index) const
	{
		if (index < _begin_index) {
			return 0;
		} else if (index < _end_index) {
			return _end_index - index;
		} else {
			return 0;
		}
	}

  public:
	/**
	 * Convert to bool to check page validity
	 *
	 * @return @c true if the page is valid; @c false otherwise
	 */
	operator bool() const { return (_base_address != nullptr); }

  private:
	void* _base_address;
	size_t _begin_index;
	size_t _end_index;
	size_t _size;
};

} // namespace pvcop::core

} // namespace pvcop

#endif // __PVCOP_CORE_MEMPAGE_H__
