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

#ifndef PVCOP_TYPES_FORMATTER_INTERFACE_H
#define PVCOP_TYPES_FORMATTER_INTERFACE_H

#include <memory>

#include <pvcop/db/types.h>
#include <pvcop/db/write_dict.h>
#include <pvcop/db/read_dict.h>

namespace pvcop
{

class collector;
class collection;

namespace core
{
class mempage;
}

namespace types
{

/**
 * This class defines the interface to extract and format values from and to
 * a null-terminated string.
 */
class formatter_interface
{
	friend class pvcop::collector;  // for invalid values 'read' and
	friend class pvcop::collection; // 'write' dictionnaries handling

  public:
	static constexpr size_t MAX_STRING_LENGTH = 4096;

  public:
	using shared_ptr = std::shared_ptr<formatter_interface>;
	using sp = shared_ptr;

  public:
	/**
	 * Default constructor
	 */
	formatter_interface() {}

	/**
	 * Destructor
	 */
	virtual ~formatter_interface() {}

	formatter_interface& operator=(const formatter_interface&) = delete;
	formatter_interface(const formatter_interface&) = delete;

  public:
	virtual std::string name() const = 0;

	virtual const char* parameters() const = 0;
	virtual void set_parameters(const char*) = 0;

  public:
	/**
	 * Extract a value from a null-terminated string and store it at a
	 * dereferenced memory address.
	 *
	 * @param str the buffer to read from
	 * @param base_addr a memory address
	 * @param i an index related to the "undereferenced" memory address
	 * @param pass_autodetect return true if autodetect should pass, false otherwise
	 *
	 * @return true on success; false otherwise
	 */
	virtual bool from_string(const char* str,
	                         void* base_addr,
	                         size_t i,
	                         bool* pass_autodetect = nullptr) const = 0;

	/**
	 * Extract a value from a null-terminated string and write it at at the
	 * specified position in the provided sink.
	 *
	 * @param str the buffer to read from
	 * @param page the provided page
	 * @param row the specified row
	 *
	 * @return true on success; false otherwise
	 */
	virtual bool from_string(const char* str, core::mempage& page, size_t row) const = 0;

	/**
	 * Format a value to a null-terminated string by reading it from a
	 * dereferenced memory address.
	 *
	 * @param str a buffer to write to
	 * @param str_len the buffer size
	 * @param base_addr a memory address
	 * @param i an index related to the "undereferenced" memory address
	 *
	 * @return the written size or a negative value to report an error
	 */
	virtual int
	to_string(char* str, const size_t str_len, const void* base_addr, size_t i) const = 0;

	/**
	 * Ensure underlying value is initialized to default value (0)
	 *
	 * @param page the provided page
	 * @param row the specified row
	 */
	virtual void set_default_value(core::mempage& page, size_t row) const = 0;

  protected:
	void set_invalid_write_dict(db::write_dict* dict) { _invalid_write_dict = dict; }

  protected:
	db::write_dict* _invalid_write_dict = nullptr;
};

} // namespace pvcop::types

} // namespace pvcop

#endif // PVCOP_TYPES_FORMATTER_INTERFACE_H
