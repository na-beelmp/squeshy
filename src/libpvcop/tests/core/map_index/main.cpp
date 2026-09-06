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

#include <pvcop/core/map_index.h>

#include <pvlogger.h>

#include <vector>
#include <limits>
#include <algorithm>
#include <random>

/**
 * Test map_index class.
 * - parallel insertion
 * - find call
 * - iteration
 */

/* The type used as key to test pvcop::core::map_index; as the values will be used as an index in
 * an array, they are necessarily positive.
 */
using type_t = uint16_t;

using index_u16_t = pvcop::core::map_index<type_t, size_t>;

int main()
{
	std::vector<type_t> data(10000000);
	std::vector<size_t> ref_histo(std::numeric_limits<type_t>::max(), 0);
	std::vector<size_t> indices(data.size());
	const size_t max_value = std::numeric_limits<type_t>::max() / 2;

	/* initializing some random values and making sure the value range is
	 * not fully represented.
	 */
	std::mt19937 gen(0);
	std::uniform_int_distribution<type_t> dis(0, max_value);

	for (size_t i = 0; i < data.size(); ++i) {
		const type_t v = dis(gen);
		data[i] = v;
		ref_histo[v] += 1;
	}

	/* need to know how many different values we have generated
	 */
	size_t distinct_count =
	    std::count_if(ref_histo.begin(), ref_histo.end(), [](size_t v) { return v != 0; });

	/* time to fill our map_index in a stressful way: using non-static really
	 * small data partitioning.
	 */
	index_u16_t index;

#pragma omp parallel for schedule(dynamic, 10)
	for (size_t i = 0; i < data.size(); ++i) {
		indices[i] = index.insert(data[i]);
	}

	/* time to check map_index geometry
	 */
	if (distinct_count != index.size()) {
		pvlogger::error() << "have found " << index.size() << " distinct values, should be "
		                  << distinct_count << std::endl;
		return 1;
	}

	/* time to check correctness of the map_index content using the iterators
	 */
	std::vector<size_t> read_dict(index.size());

	for (const auto& e : index) {
		read_dict[e.second] = e.first;
	}

	bool has_error = false;

	for (size_t i = 0; i < data.size(); ++i) {
		type_t r = data[i];
		type_t v = read_dict[indices[i]];

		if (v != r) {
			pvlogger::error() << "values differ at " << i << ": " << v << " instead of " << r
			                  << std::endl;
			has_error = true;
		}
	}

	if (has_error) {
		return 1;
	}

	has_error = false;

	/* time to check that ::exist(...) works correctly: we check for each value of
	 * the value space exist (or not) in the map_index (using the histogram)
	 */
	for (size_t i = 0; i < ref_histo.size(); ++i) {
		bool exists = ref_histo[i] != 0;
		bool in_index = index.exist(i);

		if (in_index != exists) {
			has_error = true;
			if (exists) {
				pvlogger::error() << "value " << i << "should exist" << std::endl;
			} else {
				pvlogger::error() << "unexpected value " << i << std::endl;
			}
		}
	}

	if (has_error) {
		pvlogger::error() << "error while using ::find(...) " << std::endl;
		return 1;
	}

	/* time to check that ::find(...) works correctly
	 */
	if (index.find(data[0]).second != indices[0]) {
		pvlogger::error() << "::find() does not seems to work on an existing value" << std::endl;
		return 1;
	}

	if (index.find(max_value + 1).second != index_u16_t::invalid_index) {
		pvlogger::error() << "::find() does not seems to work on a non-existing value" << std::endl;
		return 1;
	}

	/* time to check that ::index(...) works correctly: the same way than ::find(...
	 */
	if (index.index(data[0]) != indices[0]) {
		pvlogger::error() << "::index() does not seems to work on an existing value" << std::endl;
		return 1;
	}

	if (index.index(max_value + 1) != index_u16_t::invalid_index) {
		pvlogger::error() << "::index() does not seems to work on a non-existing value"
		                  << std::endl;
		return 1;
	}

	/* time to check that ::clear() works.
	 */
	index.clear();

	if (index.size() != 0) {
		pvlogger::error() << "::clear() does not seems to work" << std::endl;
		return 1;
	}

	return 0;
}
