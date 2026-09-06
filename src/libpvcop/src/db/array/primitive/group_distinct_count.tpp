//
// MIT License
//
// © ESI Group, 2015
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
//
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
//
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef __PVCOP_ARRAY_PRIMITIVE_GROUP_DISTINCT_COUNT_TPP__
#define __PVCOP_ARRAY_PRIMITIVE_GROUP_DISTINCT_COUNT_TPP__

#include <unordered_set>
#include <tbb/scalable_allocator.h>

namespace pvcop
{

namespace db
{

namespace primitive
{

template <class Array>
static core::memarray<index_t> group_distinct_count(Array const& array,
                                                    core::array<index_t> const& groups,
                                                    core::array<index_t> const& extents)
{
	pvlogger::trace() << "db::primitive::group_distinct_count(...)" << std::endl;

	core::memarray<index_t> distinct_count_array(extents.size());
	std::fill(distinct_count_array.begin(), distinct_count_array.end(), 0);

	using type_t = typename Array::value_type;
	using set = std::unordered_set<type_t, std::hash<type_t>, std::equal_to<type_t>,
	                               tbb::scalable_allocator<type_t>>;

	std::vector<set> distinct_values(extents.size());

	index_t idx = 0;
	for (typename Array::const_iterator value_it = array.begin(); value_it != array.end();
	     ++value_it, idx++) {
		distinct_values[groups[idx]].emplace(*value_it);
	}

	for (size_t i = 0; i < distinct_values.size(); i++) {
		distinct_count_array[i] = distinct_values[i].size();
	}

	return distinct_count_array;
}

} // namespace pvcop::db::primitive

} // namespace pvcop::db

} // namespace pvcop

#endif // __PVCOP_ARRAY_PRIMITIVE_GROUP_DISTINCT_COUNT_TPP__
