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
#include <pvcop/db/collection.h>

#include <pvcop/db/exceptions/invalid_collection.h>

#include <pvlogger.h>

#include <stdlib.h>

static constexpr const char* data_path = std::filesystem::temp_directory_path().string() +  "/test-pvcop-db-collection";

/**
 * WARNING: If you change the format, do not forget to update my_sink::append to fill all column
 */
static pvcop::db::format data_format = {"number_uint32", "number_uint16", "number_uint64"};

/*****************************************************************************
 * to write lots of non-zero bits
 *****************************************************************************/

size_t gen_value(size_t i)
{
	return std::numeric_limits<size_t>::max() - i;
}

/*****************************************************************************
 * our sink
 *****************************************************************************/

class my_sink : public pvcop::db::sink
{
  public:
	my_sink(pvcop::db::collector& c) : pvcop::db::sink(c) {}

	void append(size_t base_index, size_t count)
	{
		size_t local_index = base_index;
		size_t remain = count;

		while (remain) {
			size_t pcount = check_fault(local_index);

			if (pcount == 0) {
				bool ret = refresh_pages(local_index);
				if (ret == false) {
					pvlogger::error() << "requesting a new page fails: local_index=" << local_index
					                  << std::endl;
					exit(3);
				}
				pcount = check_fault(local_index);
			}
			pcount = std::min(pcount, remain);

			for (size_t i = 0; i < pcount; ++i) {
				at<uint32_t>(0, local_index + i) = gen_value(local_index + i);
				at<uint16_t>(1, local_index + i) = gen_value(local_index + i);
				at<int64_t>(2, local_index + i) = gen_value(local_index + i);
			}

			local_index += pcount;
			remain -= pcount;
		}

		increment_row_count(count);
	}
};

/*****************************************************************************
 * array validation
 *****************************************************************************/

template <typename T>
bool check_array(const pvcop::db::array& array, size_t count, size_t col)
{
	if (count == 0) {
		if ((bool)array) {
			pvlogger::error() << "col " << col << " should be null (as there is no data)"
			                  << std::endl;
			return false;
		}
		return true;
	}

	if (not array) {
		pvlogger::error() << "col " << col << " should not be null" << std::endl;
		return false;
	}

	if (array.type() != pvcop::db::type_traits::type<T>::get_type_id()) {
		pvlogger::error() << "col " << col << " is typed "
		                  << pvcop::db::type_traits(array.type()).get_name() << " instead of "
		                  << pvcop::db::type_traits::type<T>::get_name() << std::endl;
		return false;
	}

	pvcop::core::array<T> native_array = array.to_core_array<T>();

	if (not native_array) {
		pvlogger::error() << "col " << col << " should be convertible to core::array<"
		                  << pvcop::db::type_traits::type<T>::get_name() << ">" << std::endl;
		return false;
	}

	bool has_error = false;
	for (size_t i = 0; i < count; ++i) {
		if (native_array[i] != static_cast<T>(gen_value(i))) {
			has_error = true;
			pvlogger::error() << "data[" << col << "][" << i << "] == " << native_array[i]
			                  << " instead of " << (T)gen_value(i) << std::endl;
		}
	}

	if (has_error) {
		return false;
	}

	return true;
}

/*****************************************************************************
 * array validation demultiplexer
 *****************************************************************************/

void check_array(const pvcop::db::array& a, pvcop::db::type_t t, size_t row_count, size_t col)
{
	bool is_ok = true;

	switch (t) {
	case pvcop::db::type_bool:
		is_ok = check_array<bool>(a, row_count, col);
		break;
	case pvcop::db::type_int8:
		is_ok = check_array<int8_t>(a, row_count, col);
		break;
	case pvcop::db::type_int16:
		is_ok = check_array<int16_t>(a, row_count, col);
		break;
	case pvcop::db::type_int32:
		is_ok = check_array<int32_t>(a, row_count, col);
		break;
	case pvcop::db::type_int64:
		is_ok = check_array<int64_t>(a, row_count, col);
		break;
	case pvcop::db::type_uint8:
		is_ok = check_array<uint8_t>(a, row_count, col);
		break;
	case pvcop::db::type_uint16:
		is_ok = check_array<uint16_t>(a, row_count, col);
		break;
	case pvcop::db::type_uint32:
		is_ok = check_array<uint32_t>(a, row_count, col);
		break;
	case pvcop::db::type_uint64:
		is_ok = check_array<uint64_t>(a, row_count, col);
		break;
	case pvcop::db::type_float:
		is_ok = check_array<float>(a, row_count, col);
		break;
	case pvcop::db::type_double:
		is_ok = check_array<double>(a, row_count, col);
		break;
	default:
		assert(false && "Not real type, they should not be checked");
	}

	if (is_ok == false) {
		pvlogger::error() << "error with column " << col << std::endl;
	}
}

/*****************************************************************************
 * main
 *****************************************************************************/

#define ROW_PER_BLOCK 100
#define BLOCK_COUNT 100

int main()
{
	// Start with 1 row as Collection without row doesn't make sens
	for (size_t row_count = 1; row_count < ROW_PER_BLOCK; ++row_count) {

		// initializing the data with a fresh collector
		{
			pvcop::db::collector ctor(data_path, data_format);

			if (ctor == false) {
				pvlogger::error() << "fails opening the collector" << std::endl;
				return 1;
			}

#pragma omp parallel
			{
				my_sink s(ctor);

// process one block at a time (but in parallel:)
#pragma omp for schedule(static, 1)
				for (size_t i = 0; i < BLOCK_COUNT; ++i) {
					s.append(i * row_count, row_count);
				}
			}

			ctor.close();

			if (ctor == true) {
				pvlogger::error() << "collector should be closed" << std::endl;
				return 3;
			}
		}

		// testing the collection
		{
			size_t real_row_count = row_count * BLOCK_COUNT;

			pvcop::db::collection ction(data_path);

			// check for an opened collection
			if (ction == false) {
				pvlogger::error() << "fails to open the collection" << std::endl;
				return 4;
			}

			// check for collection geometry: column count
			if (ction.column_count() != data_format.size()) {
				pvlogger::error() << "collection has " << ction.column_count()
				                  << " column(s) instead of " << data_format.size() << std::endl;
				return 5;
			}

			// check for collection geometry: row count
			if (ction.row_count() != real_row_count) {
				pvlogger::error() << "collection has " << ction.row_count() << " rows instead of "
				                  << real_row_count << std::endl;
				return 6;
			}

			// check for the format known by the collection
			for (size_t c = 0; c < data_format.size(); ++c) {
				if (ction.type(c) != data_format[c]) {
					pvlogger::error()
					    << "column  " << c << " is typed "
					    << pvcop::db::type_traits(ction.type(c)).get_name() << " instead of "
					    << pvcop::db::type_traits(data_format[c]).get_name() << std::endl;
					return 7;
				}
			}

// check for data integrity
#pragma omp parallel for
			for (size_t c = 0; c < data_format.size(); ++c) {
				check_array(ction.column(c), data_format[c], real_row_count, c);
			}

			{
				// check for multiple instances existence (for non-null collection:)
				pvcop::db::array dba = ction.column(0);

				pvcop::core::array<uint32_t> ca = dba.to_core_array<uint32_t>();

#define MANY_ACCESS_COUNT 50

				uint32_t accum_ref = ca[0] * MANY_ACCESS_COUNT;
				uint32_t accum = 0;

#pragma omp parallel for schedule(static, 1) reduction(+ : accum)
				for (size_t i = 0; i < MANY_ACCESS_COUNT; ++i) {
					pvcop::db::array dba2 = ction.column(0);
					pvcop::core::array<uint32_t> ca2 = dba2.to_core_array<uint32_t>();

					if (ca2.data() != ca.data()) {
						pvlogger::error() << "an other instance has different address"
						                  << " index is " << i << " taken one is "
						                  << static_cast<void*>(ca2.data()) << " correct one is "
						                  << static_cast<void*>(ca.data()) << std::endl;
						exit(9);
					}

					accum += ca2[0];
				}

				if (accum != accum_ref) {
					pvlogger::error() << "parallel accesses computation error: " << accum
					                  << " instead of " << accum_ref << std::endl;
					return 10;
				}
			}

			ction.close();

			if (ction == true) {
				pvlogger::error() << "collection should be closed" << std::endl;
				return 11;
			}
		}
	}

	{
		/* check that db::exception::invalid_collection is thrown on a nonexistent directory
		 */
		try {
			pvcop::db::collection(std::filesystem::temp_directory_path().string() +  "/theorically_missing_dir");
			return 1;
		} catch (pvcop::db::exception::invalid_collection&) {
			// catching it with no code to accept it
		}
	}

	return 0;
}
