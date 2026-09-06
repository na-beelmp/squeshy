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

#include "string_perf.h"
#include "string_set.h"

#include <pvcop/core/map_index.h>
#include <common/squey_assert.h>

#include <pvcop/db/impl/write_dict_types.h>

#include <iostream>
#include <chrono>
#include <cstdlib>

void do_string_perf(const char* filename, size_t expected_size)
{
	string_set ss(filename);

	pvcop::core::map_index<pvcop::db::__impl::write_dict_string_t, size_t,
	                       pvcop::db::__impl::write_dict_string_hashcompare_t>
	    dict;

	const string_set::entries_t& entries = ss.entries();

	auto start = std::chrono::system_clock::now();

#pragma omp parallel for
	for (size_t i = 0; i < entries.size(); ++i) {
		dict.insert(entries[i]);
	}

	auto end = std::chrono::system_clock::now();

	std::chrono::duration<double> diff = end - start;
	std::cout << diff.count();

	PV_VALID(dict.size(), expected_size);
}
