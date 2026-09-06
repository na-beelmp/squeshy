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

#ifndef __PVCOP_DB_TYPES_TRAITS_H__
#define __PVCOP_DB_TYPES_TRAITS_H__

namespace pvcop
{

namespace db
{

/**
 * This class is used by the types_manager to resolve calls
 * to templated type functions from a type_id.
 */
class traits_interface
{
  public:
	virtual ~traits_interface(){};

	virtual size_t to_mem_size(const size_t count) const = 0;

	virtual size_t from_mem_size(const size_t count) const = 0;
};

template <typename T>
class traits : public traits_interface
{
  public:
	size_t to_mem_size(const size_t count) const override
	{
		return pvcop::core::__impl::type_traits<T>::to_mem_size(count);
	}

	size_t from_mem_size(const size_t count) const override
	{
		return pvcop::core::__impl::type_traits<T>::from_mem_size(count);
	}
};

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_TYPES_TRAITS_H__
