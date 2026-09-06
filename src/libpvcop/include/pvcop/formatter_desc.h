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

#ifndef __PVCOP_FORMATTER_DESC_H__
#define __PVCOP_FORMATTER_DESC_H__

#include <string>

namespace pvcop
{

/**
 * This class aims to abstract the formatters when setting or changing the formatters to
 * use with pvcop::collector or pvcop::collection.
 */
class formatter_desc
{
  public:
	/**
	 * constructor
	 *
	 * @param name the formatter name
	 * @param parameters the formatter parameters string
	 */
	formatter_desc(const std::string& name, const std::string& parameters)
	    : _name(name), _parameters(parameters)
	{
	}

  public:
	/**
	 * Gets the formatter name
	 *
	 * @return the formatter name
	 */
	const std::string& name() const { return _name; }

	/**
	 * Gets the formatter parameters string
	 *
	 * @return the formatter parameters string
	 */
	const std::string& parameters() const { return _parameters; }

  public:
	/**
	 * Tests 2 formatter_desc for equality
	 *
	 * @param rhs the formatter_desc to compare to
	 *
	 * @return true if they are equal; false otherwise.
	 */
	bool operator==(const formatter_desc& rhs) const
	{
		return (name() == rhs.name()) && (parameters() == rhs.parameters());
	}

	/**
	 * Tests 2 formatter_desc for inequality
	 *
	 * @param rhs the formatter_desc to compare to
	 *
	 * @return true if they are different; false otherwise.
	 */
	bool operator!=(const formatter_desc& rhs) const { return not(*this == rhs); }

  private:
	std::string _name;
	std::string _parameters;
};

} // namespace pvcop

#endif // __PVCOP_FORMATTER_DESC_H__
