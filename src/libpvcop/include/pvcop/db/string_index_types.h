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

#ifndef __PVCOP_DB_STRING_INDEX_TYPES_H__
#define __PVCOP_DB_STRING_INDEX_TYPES_H__

#define DEFINE_INDEX_TYPE(NAME, TYPE)                                                              \
	struct NAME {                                                                                  \
                                                                                                   \
		NAME() : v(0) {}                                                                           \
		NAME(TYPE v_) : v(v_) {}                                                                   \
		using type = TYPE;                                                                         \
                                                                                                   \
		inline operator TYPE() const { return v; }                                                 \
                                                                                                   \
		inline NAME& operator=(const TYPE& rhs)                                                    \
		{                                                                                          \
			v = rhs;                                                                               \
			return *this;                                                                          \
		}                                                                                          \
                                                                                                   \
		TYPE v;                                                                                    \
	};                                                                                             \
                                                                                                   \
	namespace std                                                                                  \
	{                                                                                              \
                                                                                                   \
	template <>                                                                                    \
	struct is_arithmetic<NAME> : public is_arithmetic<TYPE> {                                      \
	};                                                                                             \
                                                                                                   \
	template <>                                                                                    \
	struct is_integral<NAME> : public is_integral<TYPE> {                                          \
	};                                                                                             \
                                                                                                   \
	template <>                                                                                    \
	struct is_signed<NAME> : public is_signed<TYPE> {                                              \
	};                                                                                             \
                                                                                                   \
	template <>                                                                                    \
	struct common_type<unsigned long int, NAME> {                                                  \
		using type = unsigned long int;                                                            \
	};                                                                                             \
                                                                                                   \
	template <>                                                                                    \
	struct hash<NAME> : public hash<TYPE> {                                                        \
	};                                                                                             \
                                                                                                   \
	template <>                                                                                    \
	struct numeric_limits<NAME> : public numeric_limits<TYPE> {                                    \
	};                                                                                             \
	}                                                                                              \
                                                                                                   \
	using string_index_t = NAME;

#ifdef INDEXES_64BITS
DEFINE_INDEX_TYPE(string_index_64, uint64_t)
#else
DEFINE_INDEX_TYPE(string_index_32, uint32_t)
#endif

#endif // __PVCOP_DB_STRING_INDEX_TYPES_H__
