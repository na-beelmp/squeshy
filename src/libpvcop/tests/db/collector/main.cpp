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

#include <cstddef>
#include <cstdint>
#include <memory>

#include <pvlogger.h>

#include <chrono>
#include <filesystem>

class my_sink : public pvcop::db::sink
{
  public:
	my_sink(pvcop::db::collector& c) : pvcop::db::sink(c), _index(0) {}

	void append(uint32_t* a, uint16_t* b, size_t count)
	{
		size_t local_index = 0;

		size_t remain = count;

		while (remain) {
			size_t pcount = check_fault(_index + local_index);

			if (pcount == 0) {
				bool ret = refresh_pages(_index + local_index);
				if (ret == false) {
					pvlogger::error() << "requesting a new page fails" << std::endl;
				}
				assert(ret == true);
				pcount = check_fault(_index + local_index);
				assert(pcount != 0);
			}
			pcount = std::min(pcount, remain);

			for (size_t i = 0; i < pcount; ++i) {
				at<uint32_t>(0, _index + local_index + i) = a[local_index + i];
				at<uint16_t>(1, _index + local_index + i) = b[local_index + i];
			}

			local_index += pcount;
			remain -= pcount;
		}

		_index += count;
		increment_row_count(count);
	}

	/*void append(uint32_t a, uint16_t b)
	{
	        refresh_pages(_index);

	        at<uint32_t>(0, _index) = a;
	        at<uint32_t>(1, _index) = b;

	        ++_index;
	        increment_row_count(1);
	}*/

	bool check_types(pvcop::db::format& f)
	{
		for (size_t i = 0; i < f.size(); ++i) {
			const pvcop::db::type_t ft = f[i];
			const pvcop::db::type_t st = type(i);
			if (ft != st) {
				pvlogger::error() << "in my_sink, column " << i << " has type " << st
				                  << " instead of " << ft << std::endl;
				return false;
			}
		}
		return true;
	}

  private:
	size_t _index;
};

int main()
{
	pvcop::db::format f = {"number_uint32", "number_uint16"};

	// The path has to be unique across the testsuite: ctest runs the tests in
	// parallel, and a collector truncating a file another one still has mapped
	// faults it with SIGBUS.
	std::string collector_path =
	    std::filesystem::temp_directory_path().string() + "/collector_test_db_collector";
	pvcop::db::collector c(collector_path.c_str(), f);

	if (c) {
		pvlogger::info() << "collector initialized" << std::endl;
	} else {
		pvlogger::error() << "collector not properly initialized" << std::endl;
		return 1;
	}

	{
		my_sink s1(c);

		if (s1.check_types(f) == false) {
			return 1;
		}

		constexpr size_t count = 2 * 1024 * 1024;
		std::unique_ptr<uint32_t[]> col0(new uint32_t[count]);
		std::unique_ptr<uint16_t[]> col1(new uint16_t[count]);

		for (size_t i = 0; i < count; ++i) {
			col0.get()[i] = 2 * i;
			col1.get()[i] = 2 * i + 1;
		}

		auto start = std::chrono::system_clock::now();
		size_t num = 0;
		for (size_t i = 0; i < 10; ++i) {
			s1.append(col0.get(), col1.get(), count);
			num += count;
		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = end - start;
		double dt = diff.count();
		pvlogger::info() << "writing " << num << " rows in " << dt << " -> " << num / dt << " rps"
		                 << std::endl;
	}

	c.close();

	if (c == false) {
		pvlogger::info() << "collector properly closed" << std::endl;
	} else {
		pvlogger::error() << "collector not properly closed" << std::endl;
		return 1;
	}

	return 0;
}
