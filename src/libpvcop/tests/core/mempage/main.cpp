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

#include <pvcop/core/mempage.h>

#include <pvlogger.h>
#include <common/squey_assert.h>

int main()
{
	pvcop::core::mempage page;

	/* test of nul-page
	 */
	PV_VALID(page.base(), (void*)0);
	PV_VALID(page.size(), (size_t)0);
	PV_VALID(page.operator bool(), false);

	PV_VALID(page.relative_index(42), (size_t)42);

	PV_VALID(page.check(0), false);
	PV_VALID(page.check(1), false);

	PV_VALID(page.check_fault(0), (size_t)0);
	PV_VALID(page.check_fault(1), (size_t)0);

	/* test of "first allocated" page
	 */
	const size_t mem_len = 1024;
	const size_t int_len = mem_len / sizeof(int);
	int buffer[mem_len];

	page = pvcop::core::mempage((void*)buffer, 0, int_len);

	PV_VALID(page.base(), (void*)buffer);
	PV_VALID(page.size(), int_len);
	PV_VALID(page.operator bool(), true);

	PV_VALID(page.relative_index(0), (size_t)0);
	PV_VALID(page.relative_index(42), (size_t)42);
	PV_VALID(page.relative_index(int_len - 1), int_len - 1);
	PV_VALID(page.relative_index(int_len), int_len);

	PV_VALID(page.check(0), true);
	PV_VALID(page.check(42), true);
	PV_VALID(page.check(int_len - 1), true);
	PV_VALID(page.check(int_len), false);

	PV_VALID(page.check_fault(0), int_len);
	PV_VALID(page.check_fault(42), int_len - 42);
	PV_VALID(page.check_fault(int_len - 1), (size_t)1);
	PV_VALID(page.check_fault(int_len), (size_t)0);

	/* test of "next allocated" page
	 */
	page = pvcop::core::mempage((void*)buffer, int_len, int_len);

	PV_VALID(page.base(), (void*)buffer);
	PV_VALID(page.size(), int_len);
	PV_VALID(page.operator bool(), true);

	PV_VALID(page.relative_index(0), static_cast<size_t>(-int_len));
	PV_VALID(page.relative_index(42), (size_t)(-int_len + 42));
	PV_VALID(page.relative_index(int_len), (size_t)0);
	PV_VALID(page.relative_index(2 * int_len - 1), int_len - 1);
	PV_VALID(page.relative_index(2 * int_len), int_len);

	PV_VALID(page.check(0), false);
	PV_VALID(page.check(42), false);
	PV_VALID(page.check(int_len), true);
	PV_VALID(page.check(2 * int_len - 1), true);
	PV_VALID(page.check(2 * int_len), false);

	PV_VALID(page.check_fault(0), (size_t)0);
	PV_VALID(page.check_fault(42), (size_t)0);
	PV_VALID(page.check_fault(int_len), int_len);
	PV_VALID(page.check_fault(2 * int_len - 1), (size_t)1);
	PV_VALID(page.check_fault(2 * int_len), (size_t)0);

	return 0;
}
