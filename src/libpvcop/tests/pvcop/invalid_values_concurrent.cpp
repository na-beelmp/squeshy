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

/**
 * Checks that invalid values survive concurrent sinks.
 *
 * The import pipeline hands each chunk of the input to its own sink, so several
 * sinks flag rows of a same column invalid at the same time. The invalid selection
 * being a bit array, marking a row is a read-modify-write of the machine word
 * covering 64 rows: unless that write is atomic, sinks working on rows less than a
 * word apart wipe each other's bits.
 *
 * A wiped bit does not corrupt the value in place, which makes it treacherous: the
 * row merely reads back as valid, and the invalid dictionary index stored in its
 * slot gets formatted as if it were a genuine value -- a number out of nowhere, or
 * a date boost refuses to build.
 *
 * Going through the real pipeline would leave the collision to the mercy of the
 * TBB scheduler, which almost never lines two sinks up on the same word at the
 * same instant. So this test drives sinks directly, the way the pipeline does --
 * one sink per thread, each writing its own contiguous slice of rows -- but keeps
 * them on the same word at all times with a barrier. On the racy code the loss
 * then shows up in the thousands of rows; on the fixed one it never does.
 */

#include <pvcop/collection.h>
#include <pvcop/collector.h>
#include <pvcop/formatter_desc_list.h>
#include <pvcop/sink.h>

#include <common/squey_assert.h>

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

//! One sink per thread, like the pipeline runs one sink per chunk.
static constexpr size_t THREAD_COUNT = 8;
//! Rows per thread and per word: slices within a word, like chunk boundaries
//! falling in the middle of one.
static constexpr size_t ROWS_PER_THREAD = 8;
//! Rows covered by one machine word of the invalid selection.
static constexpr size_t WORD_ROW_COUNT = THREAD_COUNT * ROWS_PER_THREAD;
//! Each word is one independent chance to lose a bit.
static constexpr size_t WORD_COUNT = 4096;
static constexpr size_t ROW_COUNT = WORD_ROW_COUNT * WORD_COUNT;

//! Anything number_uint32 fails to convert, so that every row gets flagged.
static constexpr const char INVALID_VALUE[] = "not_a_number";

/* Releases all threads within nanoseconds of each other, where a blocking barrier
 * would stagger them by its wake-up latency. Threads spin first and yield only
 * after a while, so the test also keeps moving when they outnumber the cores --
 * where the loss comes from preemption in the middle of the read-modify-write
 * rather than from simultaneity.
 */
class spin_barrier
{
  public:
	explicit spin_barrier(size_t thread_count) : _thread_count(thread_count) {}

	void arrive_and_wait()
	{
		const size_t generation = _generation.load(std::memory_order_acquire);
		if (_arrived.fetch_add(1, std::memory_order_acq_rel) + 1 == _thread_count) {
			_arrived.store(0, std::memory_order_relaxed);
			_generation.fetch_add(1, std::memory_order_release);
		} else {
			size_t spins = 0;
			while (_generation.load(std::memory_order_acquire) == generation) {
				if (++spins > 1024) {
					std::this_thread::yield();
				}
			}
		}
	}

  private:
	const size_t _thread_count;
	std::atomic<size_t> _arrived{0};
	std::atomic<size_t> _generation{0};
};

int main()
{
	const std::string collector_path =
	    std::filesystem::temp_directory_path().string() + "/collector_test_invalid_concurrent";
	std::filesystem::remove_all(collector_path);

	pvcop::formatter_desc_list formatter_descs;
	formatter_descs.emplace_back("number_uint32", "");

	{
		pvcop::collector collector(collector_path.c_str(), formatter_descs);
		spin_barrier barrier(THREAD_COUNT);

		std::vector<std::thread> threads;
		for (size_t thread_index = 0; thread_index < THREAD_COUNT; thread_index++) {
			threads.emplace_back([&, thread_index]() {
				pvcop::sink sink(collector);

				std::vector<pvcop::sink::field_t> fields(
				    ROWS_PER_THREAD,
				    pvcop::sink::field_t(INVALID_VALUE, sizeof(INVALID_VALUE) - 1));

				for (size_t word = 0; word < WORD_COUNT; word++) {
					// All threads enter the same word together, so that their
					// read-modify-writes really do interleave on it.
					barrier.arrive_and_wait();

					const size_t begin_row =
					    word * WORD_ROW_COUNT + thread_index * ROWS_PER_THREAD;
					sink.write_chunk_by_row(begin_row, ROWS_PER_THREAD, fields.data());
				}
			});
		}
		for (std::thread& thread : threads) {
			thread.join();
		}

		collector.close();
	}

	pvcop::collection collection(collector_path);
	PV_VALID(collection.row_count(), ROW_COUNT);

	const pvcop::db::array& column = collection.column(0);
	PV_VALID(column.has_invalid(), pvcop::db::INVALID_TYPE::INVALID);

	// A bit lost to a concurrent read-modify-write shows up as a row claiming to
	// be valid. Count them all before asserting, as the tally tells a race from a
	// deterministic bug.
	size_t lost_row_count = 0;
	for (size_t row = 0; row < ROW_COUNT; row++) {
		if (column.is_valid(row)) {
			++lost_row_count;
		}
	}
	if (lost_row_count != 0) {
		std::cerr << lost_row_count << " of " << ROW_COUNT
		          << " rows lost their invalid bit to concurrent sinks" << std::endl;
	}
	PV_VALID(lost_row_count, size_t(0));

	// And the original string still comes back for every row.
	for (size_t row = 0; row < ROW_COUNT; row++) {
		PV_VALID(column.at(row), std::string(INVALID_VALUE), "row", row);
	}

	std::filesystem::remove_all(collector_path);

	return 0;
}
