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

#include <pvcop/types/scaled_number.h>

#include <pvlogger.h>

#include <memory>
#include <iostream>

#include <sys/time.h>

#ifdef PVCOP_BENCH
constexpr size_t SIZE = 200000000;
#else
constexpr size_t SIZE = 1000000;
#endif

static inline double now()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return (double)tv.tv_sec + (tv.tv_usec / 1000000.);
}

using formatter_bytes = pvcop::types::formatter_scaled_number<uint32_t>;
using formatter_bytes_base = pvcop::types::formatter_number<uint32_t>;

double
do_run_fu(const char* text, uint32_t value, char* result, size_t len, formatter_bytes_base& fu)
{
	double t1, dt;

	t1 = now();
	for (size_t i = 0; i < SIZE; ++i) {
		fu.to_string(result, len, &value, 0);
	}
	dt = now() - t1;
#ifndef PVCOP_BENCH
	pvlogger::info() << text << ": " << dt << " s => " << SIZE / dt << " values/s" << std::endl;
	pvlogger::info() << text << ": '" << result << "'" << std::endl;
#endif
	return dt;
}

int main()
{
	uint32_t u = 12345678;
	size_t len = 2 * 1024;
	std::unique_ptr<char[]> s(new char[len]);

#ifndef PVCOP_BENCH
	pvlogger::info() << "running for " << SIZE << " values" << std::endl;
#endif

	formatter_bytes fu("%u o", 1);
	formatter_bytes fu_ki("%u Kio", 1024);
	formatter_bytes fu_m("%u Mo", 1000 * 1000);

	double t1 = do_run_fu("1 ", u, s.get(), len, fu);
	double tk = do_run_fu("Ki", u, s.get(), len, fu_ki);
	double tm = do_run_fu("M ", u, s.get(), len, fu_m);

	std::cout << (t1 + tk + tm) << std::endl;

	return 0;
}
