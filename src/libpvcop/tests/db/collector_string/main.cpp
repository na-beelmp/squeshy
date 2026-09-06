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

#include <pvcop/db/collector.h>
#include <pvcop/db/sink.h>
#include <pvcop/db/write_dict.h>
#include <pvcop/db/collection.h>

#include <pvcop/db/types.h>

#include <pvlogger.h>

#include <vector>
#include <algorithm>
#include <random>
#include <filesystem>

static std::string data_path = std::filesystem::temp_directory_path().string() +  "/test_collector_string";

std::string type_index_t = "string";

class my_sink : public pvcop::db::sink
{
  public:
	my_sink(pvcop::db::collector& c) : pvcop::db::sink(c) { _dict = dict(0); }

	bool has_dict() const { return _dict != nullptr; }

	void insert_dict(const char* v) { _dict->insert(v); }

	void insert(size_t row, const char* value)
	{
		refresh_pages(row);
		at<string_index_t>(0UL, row) = _dict->insert(value);
		increment_row_count(1);
	}

  private:
	pvcop::db::write_dict* _dict;
};

int main()
{
#ifdef PVCOP_BENCH
	std::vector<string_index_t> orig_indices(10000000);
#else
	std::vector<string_index_t> orig_indices(1000000);
#endif

	std::vector<const char*> dict = {"GET",     "CONNECT", "POST",   "DELETE",
	                                 "PROFIND", "PUT",     "OPTIONS"};
	const uint64_t dict_size = dict.size();

	/* some random values
	 */
	std::mt19937 gen(0);
	std::uniform_int_distribution<string_index_t::type> dis(0ULL, dict_size - 1);

	for (size_t i = 0; i < orig_indices.size(); ++i) {
		orig_indices[i] = dis(gen);
	}

	{
		pvcop::db::format f = {"string"};

		pvcop::db::collector c(data_path.c_str(), f);

		{
			my_sink dict_sink(c);

			if (dict_sink.has_dict() == false) {
				pvlogger::error() << "no dictionnary found" << std::endl;
				return 1;
			}

			/* inserting all keys to make them have the same index than in dict
			 */
			for (size_t i = 0; i < dict.size(); ++i) {
				dict_sink.insert_dict(dict[i]);
			}
		}

/* filling the column, the run is necessarily determistic as all
 * the keys have already been inserted before.
 */

#ifdef PVCOP_BENCH
		auto start = std::chrono::system_clock::now();
#endif

#if __linux__
#pragma omp parallel
#endif
		{
			my_sink insert_sink(c);

#if __linux__
#pragma omp for
#endif
			for (size_t i = 0; i < orig_indices.size(); ++i) {
				insert_sink.insert(i, dict[orig_indices[i]]);
			}
		}

#ifdef PVCOP_BENCH
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = end - start;
		std::cout << diff.count();
#endif

		c.close();
	}

#ifndef PVCOP_BENCH
	pvcop::db::collection c(data_path);

	if (c.type(0) != "string") {
		pvlogger::error() << "the collection does not have string typed column" << std::endl;
		return 1;
	}
	{

		pvcop::db::array col0 = c.column(0);

		pvcop::core::array<string_index_t> data0 = col0.to_core_array<string_index_t>();

		if (data0.size() != orig_indices.size()) {
			pvlogger::error() << "column has not the right size" << std::endl;
			return -1;
		}

		bool has_error = false;

		for (size_t i = 0; i < orig_indices.size(); ++i) {
			string_index_t r = orig_indices[i];
			string_index_t v = data0[i];

			if (v != r) {
				pvlogger::error() << "values differ at " << i << ": " << v << " instead of " << r
				                  << std::endl;
				has_error = true;
			}
		}

		if (has_error) {
			pvlogger::error() << "invalid column" << std::endl;
			return 1;
		}
	}

	c.close();
#endif

	return 0;
}
