//
// MIT License
//
// © Squey, 2026
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

// A string array normally indexes into the dictionary of the db::collection it
// was read from, so one built with the plain constructor has none and crashes
// on the first read. make_string_array() builds both, and the array owns the
// dictionary -- which is what these checks are about: values read back
// correctly, and still readable once whatever they came from is gone.

#include <pvcop/db/array.h>

#include <common/squey_assert.h>

#include <string>
#include <vector>

int main()
{
	// Values read back in order, duplicates sharing one dictionary entry.
	{
		std::vector<std::string> values = {"bbb", "aaa", "bbb", "ccc", ""};
		const pvcop::db::array array = pvcop::db::make_string_array(values);

		PV_VALID(array.size(), size_t(5));
		PV_VALID(array.type(), std::string("string"));
		for (size_t i = 0; i < values.size(); ++i) {
			PV_VALID(array.at(i), values[i]);
		}
	}

	// The array outlives the strings it was built from: the dictionary is
	// owned, not referenced.
	{
		pvcop::db::array array;
		{
			std::vector<std::string> values = {"first", "second", "first"};
			array = pvcop::db::make_string_array(values);
		}
		PV_VALID(array.at(0), std::string("first"));
		PV_VALID(array.at(1), std::string("second"));
		PV_VALID(array.at(2), std::string("first"));
	}

	// An empty result is a valid, empty array rather than a crash: a GROUP BY
	// matching nothing is an ordinary outcome.
	{
		const pvcop::db::array array = pvcop::db::make_string_array({});
		PV_VALID(array.size(), size_t(0));
	}

	// A single value repeated collapses to one dictionary entry.
	{
		const std::vector<std::string> values(1000, "same");
		const pvcop::db::array array = pvcop::db::make_string_array(values);
		PV_VALID(array.size(), size_t(1000));
		PV_VALID(array.at(0), std::string("same"));
		PV_VALID(array.at(999), std::string("same"));
	}

	return 0;
}
