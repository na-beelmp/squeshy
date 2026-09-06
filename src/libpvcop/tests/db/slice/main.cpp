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

#include <pvcop/pvcop.h>

#include <pvcop/db/types.h>

#include <iostream>
#include <limits>
#include <type_traits>

#include <common/squey_assert.h>

#include <pvlogger.h>

using namespace pvcop;

static constexpr uint16_t count = 10000;

/**
 * Fill an array with known values for floating point numbers
 */
template <typename T, typename A, bool is_float = std::is_floating_point<T>::value>
struct Initializer {
	static A& init(A& array)
	{
		auto& native_array = array.template to_core_array<T>();

		std::iota(native_array.begin(), native_array.end(), 0);

		return array;
	}
};

/**
 * Fill an array with known values for not floating point numbers
 */
template <typename T, typename A>
struct Initializer<T, A, false> {
	static A& init(A& array)
	{
		auto& native_array = array.template to_core_array<T>();

		for (size_t i = 0; i < native_array.size(); i++) {
			native_array[i] = static_cast<T>(
			    i % ((size_t)std::numeric_limits<T>::max() + std::is_same<T, bool>::value));
		}

		return array;
	}
};

/**
 * Check content of a sliced array knowing values put in the original array.
 *
 * Floating point version
 */
template <typename T, typename A, bool is_float = std::is_floating_point<T>::value>
struct ContentTester {
	static void test(A& array, size_t abs_pos)
	{
		auto& native_array = array.template to_core_array<T>();

		PV_ASSERT_VALID(native_array ? true : false);

		for (size_t i = 0; i < native_array.size(); i++) {
			PV_VALID(native_array[i] == static_cast<T>((i + abs_pos)), true, "i", i);
		}
	}
};

/**
 * Check content of a sliced array knowing values put in the original array.
 *
 * Not Floating point version
 */
template <typename T, typename A>
struct ContentTester<T, A, false> {
	static void test(A& array, size_t abs_pos)
	{
		auto& native_array = array.template to_core_array<T>();

		PV_ASSERT_VALID(native_array ? true : false);

		for (size_t i = 0; i < native_array.size(); i++) {
			PV_VALID(native_array[i] ==
			             static_cast<T>((i + abs_pos) % ((size_t)std::numeric_limits<T>::max() +
			                                             std::is_same<T, bool>::value)),
			         true, "i", i);
		}
	}
};

template <typename T, typename A>
void test_array_slicing(A& array)
{
	size_t array_size = array.size();

	// valid case : increasing length
	for (size_t i = 0; i < array.size(); i++) {
		A array_slice = array.slice(0, i);
		PV_VALID(array_slice.size() == i, true, "i", i);
	}

	// valid case : increasing position
	size_t inc = std::is_same<T, bool>::value ? 64 : 1;
	for (size_t i = 0; i < array.size(); i += inc) {
		A array_slice = array.slice(i, array.size() - i);
		PV_VALID(array_slice.size() == (array.size() - i), true, "i", i);
	}

	// warning cases : slice length exceeds array size
	for (size_t i = array_size + 1; i < array_size * 2; i++) {
		try {
			A array_slice = array.slice(0, i);
			PV_ASSERT_VALID(false);
		} catch (std::runtime_error const& e) {
		} catch (...) {
			PV_ASSERT_VALID(false);
		}
	}

	// invalid cases : starting position exceeds array size
	for (size_t i = array.size(); i < count; i++) {
		try {
			A array_slice = array.slice(i, array.size());
			PV_ASSERT_VALID(false);
		} catch (std::runtime_error const& e) {
		} catch (...) {
			PV_ASSERT_VALID(false);
		}
	}
}

template <typename T, typename A>
void test_array_subslicing(A&& array, size_t count)
{
	Initializer<T, A>::init(array);

	PV_VALID(array.size() == count, true, "array.size()", array.size(), "count", count);
	test_array_slicing<T>(array);

	ContentTester<T, A>::test(array, 0);

	size_t array_slice1_pos = (array.size() / 2) & ~63; // pos must be a multiple of 64
	size_t array_slice1_len = array.size() - array_slice1_pos;
	A array_slice1 = array.slice(array_slice1_pos, array_slice1_len);

	PV_ASSERT_VALID(array_slice1 ? true : false);
	PV_VALID(array_slice1.size() == array_slice1_len, true, "array_slice1.size()",
	         array_slice1.size(), "array_slice1_len", array_slice1_len);
	test_array_slicing<T>(array_slice1);
	ContentTester<T, A>::test(array_slice1, array_slice1_pos);

	size_t array_slice2_pos = 0;
	size_t array_slice2_len = array_slice1.size() / 2;
	A array_slice2 = array_slice1.slice(array_slice2_pos, array_slice2_len);
	PV_ASSERT_VALID(array_slice2 ? true : false);
	PV_VALID(array_slice2.size() == array_slice2_len, true, "array_slice2.size()",
	         array_slice2.size(), "array_slice2_len", array_slice2_len);
	test_array_slicing<T>(array_slice2);
	ContentTester<T, A>::test(array_slice2, array_slice1_pos + array_slice2_pos);
}

struct testsuite {
	template <typename T>
	void operator()(T) const
	{
		db::type_t type_id = pvcop::db::type_traits::type<T>::get_type_id();
		test_array_subslicing<T>(db::array(type_id, count), count);
	}
};

int main()
{
	pvcop::db::visit_supported_types(testsuite());
}
