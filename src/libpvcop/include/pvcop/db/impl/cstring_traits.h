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

#ifndef __PVCOP_DB_IMPL_CSTRING_TRAITS_H__
#define __PVCOP_DB_IMPL_CSTRING_TRAITS_H__

#include <cstring> // for strcmp

namespace pvcop
{

namespace db
{

namespace __impl
{

/**
 * Checks for the equality of 2 C strings.
 *
 * @param lhs the first string
 * @param rhs the other string
 *
 * @return true if the 2 string are strictly equal; false otherwise.
 */
static bool cstring_compare(const char* lhs, const char* rhs)
{
	return strcmp(lhs, rhs) == 0;
}

/**
 * Computes the hash given a C string.
 *
 * This is the same algo as std::string hash:
 * https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.4/a01199.html#l00085
 *
 * @return the hash of @k k
 */
static size_t cstring_hash(const char* k)
{
	const char* str = k;
	size_t __result = static_cast<size_t>(14695981039346656037ULL);
	while (*str) {
		__result ^= static_cast<size_t>(*str++);
		__result *= static_cast<size_t>(1099511628211ULL);
	}
	return __result;
}

} // namespace pvcop::db::__impl

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_DB_IMPL_CSTRING_TRAITS_H__
