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

#ifndef __PVCOP_FORMATTER_DESC_LIST_H__
#define __PVCOP_FORMATTER_DESC_LIST_H__

#include <pvcop/formatter_desc.h>

#include <vector>

namespace pvcop
{

/**
 * This class is an sequence of pvcop::formatter_desc to use
 */
class formatter_desc_list
{
  public:
	static constexpr const char* filename = "formatters";

  public:
	using formatter_descs_t = std::vector<formatter_desc>;
	using const_reference = formatter_descs_t::const_reference;
	using const_iterator = formatter_descs_t::const_iterator;

  public:
	/**
	 * Default constructor
	 */
	formatter_desc_list();

	/**
	 * File based constructor
	 *
	 * Creates and initializes an instance using the data from
	 * a file
	 *
	 * @param filename the file name to load from
	 *
	 * @throw std::io_base::failure in case of error
	 */
	explicit formatter_desc_list(const std::string& filename);

  public:
	/**
	 * Gets number of element
	 *
	 * @return the number of formatter_desc
	 */
	size_t size() const { return _formatter_descs.size(); }

	/**
	 * Convert to bool to check validity
	 *
	 * @return @c true if it contains a sequence of formatter_desc; @c false otherwise
	 */
	operator bool() const { return size() != 0; }

  public:
	/**
	 * Appends a new formatter_desc
	 *
	 * @p value is copied.
	 *
	 * @param value the new formatter_desc to append
	 */
	void push_back(const formatter_desc& value) { _formatter_descs.push_back(value); }

	/**
	 * Appends a new formatter_desc
	 *
	 * The formatter_desc is constructed from a list of compatible constructor parameters.
	 */
	template <class... Args>
	void emplace_back(Args&&... args)
	{
		_formatter_descs.emplace_back(std::forward<Args>(args)...);
	}

	/**
	 * Erase a formatter_desc at the specified position
	 */
	void erase(formatter_descs_t::const_iterator it) { _formatter_descs.erase(it); }

	/**
	 * Clear the sequence
	 */
	void clear() { _formatter_descs.clear(); }

  public:
	/**
	 * Gets a reference to the element at specified location @p pos.
	 *
	 * No bounds checking is performed.
	 *
	 * @return a reference to the element at specified location @p pos
	 */
	const_reference operator[](size_t pos) const { return _formatter_descs[pos]; }

	/**
	 * Gets a reference to the element at specified location @p pos.
	 *
	 * Bounds checking is performed.
	 *
	 * @return a reference to the element at specified location @p pos
	 *
	 * @thow std::out_of_range
	 */
	const_reference at(size_t pos) const { return _formatter_descs.at(pos); }

  public:
	/**
	 * Gets an iterator to the first element.
	 *
	 * @return an iterator to the first element.
	 */
	const_iterator begin() const { return _formatter_descs.begin(); }

	/**
	 * Gets an iterator to the element following the last element.
	 *
	 * @return an iterator to the element following the last element.
	 */
	const_iterator end() const { return _formatter_descs.end(); }

  public:
	/**
	 * Loads a list for formatter_desc from a file
	 *
	 * @param filename the file name to load from
	 *
	 * @throw std::ios::failure in case of error
	 */
	void load(const std::string& filename);

	/**
	 * Saves the list for formatter_desc into a file
	 *
	 * @param filename the file name to save to
	 *
	 * @throw std::ios::failure in case of error
	 */
	void save(const std::string& filename) const;

  public:
	/**
	 * Tests 2 formatter_desc_list for equality
	 *
	 * @param rhs the formatter_desc_list to compare to
	 *
	 * @return true if they are equal; false otherwise.
	 */
	bool operator==(const formatter_desc_list& rhs) const;

	/**
	 * Tests 2 formatter_desc_list for inequality
	 *
	 * @param rhs the formatter_desc_list to compare to
	 *
	 * @return true if they are different; false otherwise.
	 */
	bool operator!=(const formatter_desc_list& rhs) const;

  private:
	formatter_descs_t _formatter_descs;
};

} // namespace pvcop

#endif // __PVCOP_FORMATTER_DESC_LIST_H__
