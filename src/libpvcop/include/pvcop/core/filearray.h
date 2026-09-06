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

#ifndef __PVCOP_CORE_FILEARRAY_H__
#define __PVCOP_CORE_FILEARRAY_H__

#include <pvcop/core/array.h>

#include <functional>

namespace pvcop
{

namespace core
{

/**
 * The callback function type used to trigger an action when an instance of
 * core::filearray is destructed.
 */
using filearray_release_f = std::function<void(void)>;

/**
 * This class extends core::array to access a file content which is mapped in
 * memory.
 *
 * The file management has been excluded from this class to provide a versatile
 * class which permits to:
 * - avoid using derived classes and/or virtual table to have different file
 *   management logic/policy,
 * - have autonomous objects after their creation as core::filearray_release_f
 *   hides the resource release logic,
 * - have strict module/namespace partition to avoid implementation exposure
 *   and permit easy factories writing.
 *
 * @note this class is defined as internal because it is in theory only
 * needed to create a db::array whose data come from a db::collection. It
 * can be made public if there is a reason to.
 */

template <typename T>
class filearray : public array<T>
{
  public:
	using array_t = array<T>;
	using filearray_t = filearray<T>;

	using value_type = typename __impl::type_traits<T>::value_type;

  public:
	/**
	 * Default constructor
	 */
	filearray() : array_t() {}

	/**
	 * Constructor
	 *
	 * @param data the base address of the data
	 * @param size the number of values
	 * @param f the file release function
	 */
	filearray(value_type* data, size_t size, const filearray_release_f& f) : _release(f)
	{
		array_t::_data = data;
		array_t::_size = size;
	}

	/**
	 * No copy constructor
	 */
	filearray(filearray_t&) = delete;

	/**
	 * Move constructor
	 */
	filearray(filearray_t&& other) : array_t(std::move(other))
	{
		_release = std::move(other._release);
		other._release = []() {};
	}

	/**
	 * Destructor
	 */
	~filearray()
	{
		if (array_t::_data != nullptr) {
			array_t::_data = nullptr;
			array_t::_size = 0;
			_release();
		}
	}

  public:
	/**
	 * No copy assignment operator
	 */
	filearray_t& operator=(filearray_t&) = delete;

	/**
	 * No move assignment operator
	 */
	filearray_t& operator=(filearray_t&&) = delete;

  private:
	filearray_release_f _release;
};

} // namespace pvcop::core

} // namespace pvcop

#endif // __PVCOP_CORE_FILEARRAY_H__
