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

#include "export_csv.h"

#include <pvcop/collection.h>
#include <pvcop/core/memarray.h>

#include <fstream>
#include <filesystem>

#include <omp.h>

int export_csv(const char* collection_path, const char* exported_collection_path, bool parallel_run)
{
	pvcop::collection ction(collection_path);

	// check if the collection was sucessfully opened
	if (not ction) {
		pvlogger::error() << "failed to open the collection" << std::endl;
		return 1;
	}

	pvcop::core::memarray<bool> selection(ction.row_count());
	for (size_t i = 0; i < selection.size(); i++) {
		selection[i] = true; // TODO : fix core::memarray<bool> iterator !!!!
	}

	std::vector<pvcop::db::array> arrays;
	arrays.reserve(ction.column_count());
	for (size_t col = 0; col < ction.column_count(); col++) {
		arrays.push_back(ction.column(col));
	}

	std::ofstream output_file(std::filesystem::path(exported_collection_path), std::ios::binary);

	auto start = std::chrono::system_clock::now();

	if (parallel_run) {
		constexpr size_t chunk_size = 4096;
		for (size_t base_row = 0; base_row < ction.row_count(); base_row += chunk_size) {

			size_t max_row_count = std::min(chunk_size, ction.row_count() - base_row);

			std::atomic<int> ordered(0);

// FIXME This parallel add synchronisation between chunks. It is not
// required but it is harder to unsure ordered reduction otherwise.
#pragma omp parallel
			{
				std::stringstream row_stream;
#pragma omp for nowait schedule(static)
				for (size_t row = base_row; row < base_row + max_row_count; row++) {
					if (selection[row]) {
						for (size_t col = 0; col < ction.column_count(); col++) {
							row_stream << arrays[col].at(row)
							           << ((col < ction.column_count() - 1) ? "," : "\n");
						}
					}
				}

				while (ordered != omp_get_thread_num())
					;
				// FIXME : We use this barrier to make an ordered reduction. It may
				// be written using custom reduction from OpenMP 4.0 and ordered may
				// may be more scalable using array of bool to inform only the
				// neighbor when the critical section is completed.

				output_file << row_stream.str();

				ordered++;
			}
		}
	} else {
		// SEQUENTIAL
		for (size_t row = 0; row < ction.row_count(); row++) {
			if (selection[row]) {
				for (size_t col = 0; col < ction.column_count(); col++) {
					output_file << arrays[col].at(row)
					            << ((col < ction.column_count() - 1) ? "," : "\n");
				}
			}
		}
	}

	std::chrono::duration<double> time_taken = std::chrono::system_clock::now() - start;
	std::cout << "export took : " << time_taken.count() << " sec" << std::endl;

	return 0;
}
