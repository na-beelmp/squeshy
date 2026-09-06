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

#ifndef __PVCOP_CORE_IMPL_MEMORY_H__
#define __PVCOP_CORE_IMPL_MEMORY_H__

#include <cstddef>
#include <cstdlib>
#include <string.h>

namespace pvcop
{

namespace core
{

namespace __impl
{

/**
 * allocate a memory block
 *
 * @param size the size in bytes.
 *
 * @return the allocated memory block
 */
template <class T>
T* mem_allocate(size_t size, bool init)
{
	// No need to use aligned_alloc on 16 bits for SSE as libc malloc already do it.
	// Using normal malloc permit to use realloc without real reallocation.
	// http://www.gnu.org/software/libc/manual/html_node/Aligned-Memory-Blocks.html
#ifdef _WIN32
	init = true; // force initialization on Windows
#endif
	if (init) {
		return static_cast<T*>(calloc(size, sizeof(T)));
	} else {
		return static_cast<T*>(malloc(size));
	}
}

/**
 * reallocate a memory block
 *
 * @param addr the old memory block base address
 * @param new_size the new memory size in byte.
 *
 * @return the reallocated memory block
 */
template <class T>
T* mem_reallocate(T* addr, size_t new_size)
{
	void* ptr = realloc(static_cast<void*>(addr), new_size);
    return reinterpret_cast<T*>(ptr);
}

/**
 * free memory block
 *
 * @param addr the memory block base address
 */
template <class T>
void mem_deallocate(T* addr)
{
	free(addr);
}

} // namespace pvcop::core::__impl

} // namespace pvcop::core

} // namespace pvcop

#endif // __PVCOP_CORE_IMPL_MEMORY_H__
