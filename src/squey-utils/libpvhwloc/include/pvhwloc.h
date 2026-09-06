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

#ifndef __PVHWLOC__
#define __PVHWLOC__

#include <vector>

#include <stdlib.h>

/**
 * \class pvhwloc
 *
 * \note This class gives hardware information using hwloc library.
 *
 */
class pvhwloc
{
  public:
	/*! \brief Returns the number of physical cores.
	 *
	 *  \return Physical core number.
	 */
	static inline size_t core_count() { return get()._core_count; }

	/*! \brief Returns the number of logical cores.
	 *
	 *  \note a logical core -or thread- usually shares all ressources but registers.
	 *
	 *  \return Physical logical number.
	 */
	static inline size_t thread_count() { return get()._thread_count; }

	/*! \brief Check if hyperthreading is enabled.
	 *
	 *  \return True if hyperthreading is enabled, false otherwise.
	 */
	static inline bool is_hyperthreading_enabled() { return thread_count() >= 2 * core_count(); }

	/*! \brief Returns the number of cache levels.
	 *
	 *  \return The number of cache levels.
	 */
	static inline size_t cache_levels() { return get()._cache_level_sizes.size(); }

	/*! \brief Returns the size of a cache located at a given level.
	 *
	 *  \param[in] level_n The cache level. First cache is located at index 0.
	 *
	 *  \return The size of the cache if it exists, 0 otherwise.
	 */
	static inline size_t level_n_cache_size(size_t level_n)
	{
		if (level_n < get()._cache_level_sizes.size()) {
			return get()._cache_level_sizes[level_n];
		}

		return 0;
	}

  private:
	static inline pvhwloc& get()
	{
		static pvhwloc instance;

		return instance;
	}

	protected:
	pvhwloc();

  private:
	size_t _core_count;
	size_t _thread_count;
	std::vector<size_t> _cache_level_sizes;
};

#endif // __PVHWLOC__
