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

#include <pvcop/core/memarray.h>
#include <pvcop/db/types.h>
#include <pvcop/core/impl/type_traits.h>

#include <common/squey_assert.h>

#include <memory>
#include <type_traits>

static constexpr size_t BIG_NUMBER = 100000;

using namespace pvcop::core;
using namespace pvcop::core::__impl;

struct test_t {
	int i;
	long l;
	test_t(size_t v = 0) : i(v), l(v) {}

	bool operator==(const test_t& t) const { return (i == t.i) && (l == t.l); }
};

std::ostream& operator<<(std::ostream& os, test_t t)
{
	return os << "test_t(" << t.i << "," << t.l;
}

template <typename F, typename... P>
void run_check_n(const F&& f, P&&... p)
{
	for (size_t size = 0; size < BIG_NUMBER; ++size) {
		f(size, p...);
	}
}

template <typename T>
void check_default_ctor()
{
	using atype = typename std::remove_reference<typename type_traits<T>::accessor>::type;

	memarray<atype> cm;

	PV_VALID(cm.size(), 0UL);
	PV_VALID(cm.mem_size(), 0UL);

	PV_VALID(cm.begin(), static_cast<atype*>(nullptr));
	PV_VALID(cm.end(), static_cast<atype*>(nullptr));

	PV_VALID(cm.operator bool(), false);
}

template <typename T>
void check_size_ctor(size_t size)
{
	memarray<T> mat(size);
	T* data = mat.data();
	PV_VALID(mat.size(), size);
	PV_VALID(mat.mem_size(), sizeof(T) * size);
	PV_VALID(mat.begin(), data);
	PV_VALID(mat.end(), data + size);
	PV_VALID(mat.operator bool(), true);
}

template <>
void check_size_ctor<bool>(size_t size)
{
	memarray<bool> mat(size);
	bit_manip::value_type* data = mat.data();
	PV_VALID(mat.size(), size);
	PV_VALID(mat.mem_size(), bit_manip::chunk_count(size) * sizeof(bit_manip::value_type));
	bit_iterator b_begin(data, 0);
	PV_ASSERT_VALID(mat.begin() == b_begin);
	bit_iterator b_end(data, size);
	PV_ASSERT_VALID(mat.end() == b_end);
	PV_VALID(mat.operator bool(), true);
}

template <typename T>
void check_move_ctor()
{
	using atype = typename std::remove_reference<typename type_traits<T>::accessor>::type;

	memarray<atype> mat(BIG_NUMBER);
	atype* data = mat.data();
	memarray<atype> mat2(std::move(mat));
	PV_VALID(mat.size(), 0UL);
	PV_VALID(mat.data(), static_cast<atype*>(nullptr));
	PV_VALID(mat2.size(), BIG_NUMBER);
	PV_VALID(mat2.data(), data);
}

template <typename T>
void check_move_assign()
{
	using atype = typename std::remove_reference<typename type_traits<T>::accessor>::type;

	memarray<atype> mat(BIG_NUMBER);
	atype* data = mat.data();
	memarray<atype> mat2 = std::move(mat);
	PV_VALID(mat.size(), 0UL);
	PV_VALID(mat.data(), static_cast<atype*>(nullptr));
	PV_VALID(mat2.size(), BIG_NUMBER);
	PV_VALID(mat2.data(), data);
}

template <typename T>
void check_accessor()
{
	memarray<T> mat(BIG_NUMBER / 2);
	std::iota(mat.begin(), mat.end(), 0);

	for (size_t i = 0; i < BIG_NUMBER; ++i) {
		bool oor_thrown = false;
		try {
			PV_ASSERT_VALID(mat.at(i) == static_cast<T>(i));
		} catch (std::out_of_range& e) {
			oor_thrown = true;
		}
		if (i < (BIG_NUMBER / 2)) {
			PV_VALID(oor_thrown, false, "i", i);
		} else {
			PV_VALID(oor_thrown, true, "i", i);
		}
	}

	T* data = mat.data();
	for (size_t i = 0; i < BIG_NUMBER / 2; ++i) {
		PV_VALID(&mat[i], data + i, "i", i);
	}
}

template <>
void check_accessor<bool>()
{
	memarray<bool> mat(BIG_NUMBER / 2);
	bit_manip::value_type* data = mat.data();

	for (size_t i = 0; i < BIG_NUMBER; ++i) {
		bool oor_thrown = false;
		try {
			memarray<bool>::accessor acc = mat.at(i);
			bit b_ref(data, i);
			PV_VALID(acc, b_ref, "i", i);
		} catch (std::out_of_range& e) {
			oor_thrown = true;
		}
		if (i < (BIG_NUMBER / 2)) {
			PV_VALID(oor_thrown, false, "i", i);
		} else {
			PV_VALID(oor_thrown, true, "i", i);
		}
	}
	for (size_t i = 0; i < BIG_NUMBER / 2; ++i) {
		bit b_ref(data, i);
		PV_VALID(mat[i], b_ref, "i", i);
	}
}

template <typename T>
void check_iterator()
{
	memarray<T> mat(BIG_NUMBER);
	T* data = mat.data();
	PV_VALID(mat.begin(), data);
	PV_VALID(mat.end(), data + BIG_NUMBER);
}

template <>
void check_iterator<bool>()
{
	memarray<bool> mat(BIG_NUMBER);
	bit_manip::value_type* data = mat.data();

	for (size_t i = 0; i < BIG_NUMBER; i++) {
		mat[i] = i % 2;
	}

	bit_iterator b_begin(data, 0);
	PV_ASSERT_VALID(mat.begin() == b_begin);

	bit_iterator b_end(data, BIG_NUMBER);
	PV_ASSERT_VALID(mat.end() == b_end);

	size_t count = 0;
	for (auto it = mat.begin(); it != mat.end(); ++it) {
		PV_VALID(mat[count], *it);
		count++;
	}
	PV_VALID(count, BIG_NUMBER);
}

template <typename T>
void check_copy_from()
{
	memarray<T> mat(BIG_NUMBER);
	memarray<T> mat2(BIG_NUMBER);

	for (size_t i = 0; i < BIG_NUMBER; i++) {
		mat[i] = i % 5;
	}

	mat2.copy_from(mat, 0, BIG_NUMBER);

	for (size_t i = 0; i < BIG_NUMBER; i++) {
		PV_ASSERT_VALID(mat[i] == (T)(i % 5));
		PV_ASSERT_VALID(mat2[i] == (T)(i % 5));
	}
}

struct testsuite {
	template <typename T>
	void operator()(T) const
	{
		check_default_ctor<T>();
		run_check_n(check_size_ctor<T>);
		check_move_ctor<T>();
		check_move_assign<T>();
		check_accessor<T>();
		check_iterator<T>();
		check_copy_from<T>();

		// TODO: checking copy CTOR
		// TODO: checking slice CTOR from array
		// TODO: checking slice CTOR from pointer
		// TODO: checking copy_from(pointer)
		// TODO: checking copy_from(array) is trivial as it directly depends on copy_from(pointer)
	}
};

int main()
{
	pvcop::core::__impl::visit_types(testsuite(), pvcop::db::supported_types_tuple);
	pvcop::core::__impl::visit_types(testsuite(), pvcop::utils::type_tuple<test_t>());

	return 0;
}
