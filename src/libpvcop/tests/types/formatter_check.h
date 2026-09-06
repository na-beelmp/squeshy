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

#include <pvcop/pvcop.h>

#include <common/squey_assert.h>

#include <iostream>
#include <thread>
#include <random>

#include <pvlogger.h>

#include <tbb/parallel_for.h>

#include <pvcop/types/factory.h>

#ifdef PVCOP_BENCH
static constexpr size_t COUNT = 100000000; //<! Number of elements to generate and compare
#else
static constexpr size_t COUNT = 100000; //<! Number of elements to generate and compare
#endif

#define LOOP_SEQUENTIAL 0
#define LOOP_OMP 0
#define LOOP_TBB 1

/**
 * Visit in_array, apply f transformation using dtf formatter and save the result in out_array.
 *
 * It enable OpenMP, TBB and Sequential mode.
 */
template <typename T_IN, typename T_OUT, typename F>
void visit_array(T_IN& in_array,
                 const pvcop::types::formatter_interface& dtf,
                 T_OUT& out_array,
                 const F& f)
{
#if LOOP_SEQUENTIAL
	pvlogger::trace() << "(sequential loop)" << std::endl;
	pvlogger::trace() << "range=" << 0 << ".." << in_array.size() << std::endl;
	for (size_t i = 0; i < in_array.size(); i++) {

#else
	const size_t nthreads = std::thread::hardware_concurrency();
	pvlogger::trace() << "nthreads=" << nthreads << std::endl;
	std::size_t grain_size = std::max(nthreads, (in_array.size() / nthreads) & ~15);
	pvlogger::trace() << "native_array.size()=" << in_array.size() << std::endl;
	pvlogger::trace() << "grain_size=" << grain_size << std::endl << std::endl;
#endif

#if LOOP_OMP
		pvlogger::trace() << "(OMP loop)" << std::endl;
#pragma omp parallel for num_threads(nthreads) schedule(static, grain_size)
		for (size_t i = 0; i < in_array.size(); i++) {
			char str[100];
#elif LOOP_TBB
	pvlogger::trace() << "(TBB loop)" << std::endl;
    tbb::parallel_for(tbb::blocked_range<size_t>(0, in_array.size(), grain_size),
			[&](tbb::blocked_range<size_t> const& range) {
		pvlogger::trace() << "range=" << range.begin() << ".." << range.end() << std::endl;
		for (size_t i = range.begin(); i != range.end(); i++) {
#endif
			f(in_array.data(), dtf, out_array, i);
		}
#if LOOP_TBB
	}
	    , tbb::simple_partitioner());
#endif
}

/**
 * Functor to apply to_string conversion on the i-th elements of in_array_data
 */
struct format_array {
	template <typename T>
	void operator()(void* in_array_data,
	                const pvcop::types::formatter_interface& dtf,
	                T& out_array,
	                size_t i) const
	{
		static constexpr size_t str_len = 1024;
		char str[str_len];
#ifdef PVCOP_BENCH
		dtf.to_string(str, str_len, in_array_data, i);
#else
			if (not dtf.to_string(str, str_len, in_array_data, i)) {
				throw std::runtime_error("Formating impossible, invalid value : " +
				                         std::string(str, str_len));
			}
#endif
		out_array[i] = str;
	}
};

/**
 * Functor to apply from_string conversion on the i-th elements of in_array_data
 */
struct parse_array {
	template <typename T>
	void operator()(void* in_array_data,
	                const pvcop::types::formatter_interface& dtf,
	                T& out_array,
	                size_t i) const
	{
		std::string** s = reinterpret_cast<std::string**>(&in_array_data);
		const char* str = (*s)[i].c_str();
#ifdef PVCOP_BENCH
		dtf.from_string(str, out_array.data(), i);
#else
			if (not dtf.from_string(str, out_array.data(), i)) {
				throw std::runtime_error("Parsing impossible, invalid value : " + (*s)[i]);
			}
#endif
	}
};

/**
 * Generate integral values and convert it back and forth to string value using the converter and
 *check values are
 *identical.
 *
 * Formatter may not handle the full type range of value. Then, we can define min and max supported
 *values.
 */
template <typename T1, typename T2 = T1>
void bench_formatter(const pvcop::types::formatter_interface& fi,
                     T2 min = std::numeric_limits<T2>::lowest(),
                     T2 max = std::numeric_limits<T2>::max(),
                     std::function<T1(T2)> f = [](T2 t2) { return t2; })
{
	static_assert(std::numeric_limits<T2>::is_integer,
	              "Bench formatter generate random number for integral types only");

	pvcop::db::array in_array = pvcop::db::array(fi.name(), COUNT);
	pvcop::db::array out_array = pvcop::db::array(fi.name(), COUNT);
	std::vector<std::string> timestr_array(COUNT);

	auto& in_array_native = in_array.to_core_array<T1>();
	auto& out_array_native = out_array.to_core_array<T1>();

	// Fill db array with value to convert
	std::default_random_engine generator;
	std::uniform_int_distribution<T2> distribution(min, max);
	std::generate(in_array_native.begin(), in_array_native.end(), [&]() {
		T2 v = distribution(generator);
		return f(v);
	});

	// format array
	auto start = std::chrono::system_clock::now();

	visit_array(in_array, fi, timestr_array, format_array());
#ifndef PVCOP_BENCH
	auto format_time = std::chrono::system_clock::now();
	std::cout << "formatting array done in "
	          << std::chrono::duration<double>(format_time - start).count() << " sec." << std::endl;

	// parse array
	start = std::chrono::system_clock::now();
#endif
	visit_array(timestr_array, fi, out_array, parse_array());

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> diff = end - start;
#ifdef PVCOP_BENCH
	std::cout << diff.count();
#else
		std::cout << "parsing array done in " << diff.count() << " sec." << std::endl;

		// Check conversion didn't change values
		const auto& mismatch =
		    std::mismatch(in_array_native.begin(), in_array_native.end(), out_array_native.begin());
		bool equals = mismatch.first == in_array_native.end();

		PV_ASSERT_VALID(equals);
#endif
}
