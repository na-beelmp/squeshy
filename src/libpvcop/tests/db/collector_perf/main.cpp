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

#include <algorithm>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <random>
#include <filesystem>

#include <pvlogger.h>

#include <libgen.h>

/**
 * Some performance statistics:
 *
 * Hardware configuration:
 * - dual Intel Xeon E5 2620 v2 (12 cores and 24 threads)
 * - 64 Gio DDR3 RAM
 * - RAID0 on 3 SSD
 *
 * The test writes 2G uint32 (~ 7.5 Gio) under different ways:
 * - 100M rows of 20 columns
 * - 10M rows of 200 columns
 * - writing rows one by one or writing columns one by one
 * - using 1 or 6 threads (the performance decreases beyond)
 *
 * The rows are processed by chunks of 20k rows.
 *
 * Each run is done only one time.
 *
 * parameters              CPU unbound                           CPU bound binding speed-up
 * columns row/col threads time (s)  Mio/s     Me/s   speed-up   time (s)  Mio/s     Me/s speed-up
 *  20     row     1       31.362     243.268   3.188            31.975     238.599   3.127 0.981
 *  20     row     6        8.045     948.320  12.429 3.898       7.427    1027.250  13.464  4.305
 *1.083
 *  20     col     1       15.714     485.491   6.363            15.668     486.920   6.382 1.003
 *  20     col     6        6.219    1226.710  16.079 2.526       5.220    1461.390  19.155  3.001
 *1.191
 * 200     row     1       61.565     123.924   0.162            61.577     123.900   0.162 0.999
 * 200     row     6       14.053     542.889   0.711 4.381      12.955     588.904   0.771  4.753
 *1.084
 * 200     col     1       30.080     253.632   0.332            30.585     249.451   0.327 0.983
 * 200     col     6        8.280     921.404   1.207 3.638       7.869     969.514   1.271  3.887
 *1.052
 *
 * Not a big surprise: columns processing does better than rows processing,
 * even if the data::chunk is row processing friendly. It can be easily
 * explained by the fact that storage accesses must be linear and not random.
 *
 * The parallelization advantage goes to row processing. A possible
 * explanation is that column processing does less random accesses on collector
 * internal structures, leading to (proportionally) more job in kernel-space
 * when copying data (like the Linux big-locked effective memory mapping).
 *
 * Binding threads to CPUs has been tested afterward but it clearly shows the
 * (bad) effect of processes migration on processing.
 *
 * What can be enhanced/experimented:
 * - running each run many times to get a median value
 * - doing longer run could reduce standard deviation (like 10 times longer)
 * - doing measures from 1 to N threads
 * - making the chunk value quantity more constant to reflect a more realistic
 *   file processing (done generally by block instead of lines)
 * - sharing mempages between sinks instead of having mempages for each sink
 */

/*****************************************************************************
 * chunk
 *****************************************************************************/

// use a namespace because chnk is ugly as short-name :P
namespace data
{

class chunk
{
  public:
	chunk(const size_t col_num) : _col_num(col_num) {}

	~chunk()
	{
		for (auto& d : _data) {
			delete[] d;
		}
	}

	void append(uint32_t* buffer) { _data.push_back(buffer); }

	void shuffle()
	{
		// breaks a few the rows sequentiality in memory
		auto rd = std::random_device {};
		auto rng = std::default_random_engine { rd() };
		std::shuffle(_data.begin(), _data.end(), rng);
	}

	size_t col_num() const { return _col_num; }

	size_t row_num() const { return _data.size(); }

	uint32_t* at(size_t row) const { return _data[row]; }

  private:
	std::vector<uint32_t*> _data;
	size_t _col_num;
};

} // data

/*****************************************************************************
 * own pvcop::db::sink
 *****************************************************************************/

class my_sink : public pvcop::db::sink
{
  public:
	my_sink(pvcop::db::collector& c) : pvcop::db::sink(c) {}

	void append(const data::chunk& chunk, const size_t base_offset)
	{
		append(chunk, chunk.row_num(), base_offset);
	}

	void append(const data::chunk& chunk, const size_t count, const size_t base_row_offset)
	{
		size_t local_row_index = 0;
		size_t remain = count;

		while (remain) {
			size_t pcount = check_fault(base_row_offset + local_row_index);

			if (pcount == 0) {
				bool ret = refresh_pages(base_row_offset + local_row_index);
				if (ret == false) {
					pvlogger::error() << "requesting a new page fails" << std::endl;
				}
				assert(ret == true);
				pcount = check_fault(base_row_offset + local_row_index);
				assert(pcount != 0);
			}
			pcount = std::min(pcount, remain);

			for (size_t i = 0; i < pcount; ++i) {
				for (size_t j = 0; j < chunk.col_num(); ++j) {
					at<uint32_t>(j, base_row_offset + local_row_index + i) =
					    chunk.at(local_row_index + i)[j];
				}
			}

			local_row_index += pcount;
			remain -= pcount;
		}

		increment_row_count(count);
	}

	void append_col(const data::chunk& chunk, const size_t base_offset)
	{
		append_col(chunk, chunk.row_num(), base_offset);
	}

	void append_col(const data::chunk& chunk, const size_t count, const size_t base_row_offset)
	{
		for (size_t i = 0; i < chunk.col_num(); ++i) {
			size_t local_row_index = 0;
			size_t remain = count;

			while (remain) {
				size_t pcount = check_fault(i, base_row_offset + local_row_index);

				if (pcount == 0) {
					release_page(i);
					bool ret = request_page(i, base_row_offset + local_row_index);

					if (ret == false) {
						pvlogger::error() << "requesting a new page fails" << std::endl;
					}
					assert(ret == true);
					pcount = check_fault(i, base_row_offset + local_row_index);
					assert(pcount != 0);
				}
				pcount = std::min(pcount, remain);

				for (size_t j = 0; j < pcount; ++j) {
					at<uint32_t>(i, base_row_offset + local_row_index + j) =
					    chunk.at(local_row_index + j)[i];
				}

				local_row_index += pcount;
				remain -= pcount;
			}
		}

		increment_row_count(count);
	}
};

/*****************************************************************************
 * some conversion functions
 *****************************************************************************/

double get_si_size(const double s, const char*& prefix)
{
	if (s > 1000 * (double)1000) {
		prefix = "M";
		return s / (1000 * (double)1000);
	} else if (s > (double)1000) {
		prefix = "k";
		return s / (double)1000;
	} else {
		prefix = "";
		return s;
	}
}

double get_bin_size(const double s, const char*& prefix)
{
	if (s > 1024 * (double)1024) {
		prefix = "Mi";
		return s / (1024 * (double)1024);
	} else if (s > (double)1024) {
		prefix = "ki";
		return s / (double)1024;
	} else {
		prefix = "";
		return s;
	}
}

/*****************************************************************************
 * main time
 *****************************************************************************/

std::string ROOTDIR = std::filesystem::temp_directory_path().string() +  "/collector-test";
#define COL_NUM 20UL
#define ROW_NUM (1 * 1000 * 1000UL)
#define BLOCK_SIZE (20 * 1000UL)
#define THREAD_COUNT 1UL
#define WRITE_COL 0

int main(int argc, char** argv)
{
	const char* rootdir;

	size_t col_num;
	size_t row_num;
	size_t block_size;
	size_t thread_count;
	bool write_col;

	if ((argc > 1) && strcmp(argv[1], "-h") == 0) {
		printf("usage: %s [path-to-data [column-count [row-count [block-size [threads_num "
		       "[write-col]]]]]]\n",
		       basename(argv[0]));
		printf("\ndefault parameters are: %s %lu %lu %lu %lu %d\n", ROOTDIR.c_str(), COL_NUM, ROW_NUM,
		       BLOCK_SIZE, THREAD_COUNT, WRITE_COL);
		return 0;
	}

	if (argc > 1) {
		rootdir = argv[1];
	} else {
		rootdir = ROOTDIR.c_str();
	}

	if (argc > 2) {
		col_num = atol(argv[2]);
	} else {
		col_num = COL_NUM;
	}

	if (argc > 3) {
		row_num = atol(argv[3]);
	} else {
		row_num = ROW_NUM;
	}

	if (argc > 4) {
		block_size = atol(argv[4]);
	} else {
		block_size = BLOCK_SIZE;
	}

	if (argc > 5) {
		thread_count = atol(argv[5]);
	} else {
		thread_count = THREAD_COUNT;
	}

	if (argc > 6) {
		write_col = atol(argv[6]);
	} else {
		write_col = WRITE_COL;
	}

	pvcop::db::format f;

	f.resize(col_num, "number_uint32");

	pvcop::db::collector c(rootdir, f);

	if (c) {
		pvlogger::info() << "collector initialized" << std::endl;
	} else {
		pvlogger::error() << "collector not properly initialized" << std::endl;
		return 1;
	}

	pvlogger::info() << "writing a data set of " << col_num << " columns and " << row_num
	                 << " rows by block of " << block_size << " rows" << std::endl;
	pvlogger::info() << "rootdir: " << rootdir << std::endl;

	size_t num = col_num * row_num;

	data::chunk chunk(col_num);

	for (size_t row = 0; row < block_size; ++row) {
		uint32_t* b = new uint32_t[col_num];

		for (size_t col = 0; col < col_num; ++col) {
			b[col] = col;
		}

		chunk.append(b);
	}
	chunk.shuffle();

	size_t full_block_count = row_num / block_size;
	size_t partial_block_size = row_num % block_size;

	pvlogger::info() << full_block_count << " full blocks of " << block_size
	                 << " and a partial block of " << partial_block_size << std::endl;

	const char* tmp = (thread_count == 1) ? "" : "s";

	pvlogger::info() << "using " << thread_count << " thread" << tmp << std::endl;

	tmp = (write_col) ? "column" : "row";

	pvlogger::info() << "writing block as " << tmp << "-first" << std::endl;

	double dt = 0.0;
	std::chrono::time_point<std::chrono::system_clock> start;

#pragma omp parallel num_threads(thread_count)
	{
		my_sink sink(c);

// FIXME : You certainly don't measure what you want.
#pragma omp single
		start = std::chrono::system_clock::now();

#pragma omp for schedule(dynamic)
		for (size_t i = 0; i < full_block_count; ++i) {
			if (write_col) {
				sink.append_col(chunk, i * block_size);
			} else {
				sink.append(chunk, i * block_size);
			}
		}

#pragma omp single
		{
			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> diff = start - end;
			dt += diff.count();
		}
	}

	if (partial_block_size) {
		my_sink sink(c);
		start = std::chrono::system_clock::now();
		if (write_col) {
			sink.append_col(chunk, partial_block_size, full_block_count * block_size);
		} else {
			sink.append(chunk, partial_block_size, full_block_count * block_size);
		}
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = start - end;
		dt += diff.count();
	}

	const char* si_prefix;
	double si_tp = get_si_size(row_num / dt, si_prefix);

	const char* bin_prefix;
	double bin_tp = get_bin_size(num * sizeof(uint32_t) / dt, bin_prefix);

	pvlogger::info() << row_num << " events in " << dt << " s -> " << si_tp << " " << si_prefix
	                 << "e/s" << std::endl;

	pvlogger::info() << num << " octets in " << dt << " s -> " << bin_tp << " " << bin_prefix
	                 << "o/s" << std::endl;

	c.close();

	if (c == false) {
		pvlogger::info() << "collector properly closed" << std::endl;
	} else {
		pvlogger::error() << "collector not properly closed" << std::endl;
		return 1;
	}

	return 0;
}
