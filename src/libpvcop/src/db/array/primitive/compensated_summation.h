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

#ifndef __DB_ARRAY_PRIMITIVE_COMPENSATED_SUMMATION_H__
#define __DB_ARRAY_PRIMITIVE_COMPENSATED_SUMMATION_H__

namespace pvcop
{

namespace db
{

namespace primitive
{

namespace __impl
{

/**
 * taken from http://stackoverflow.com/questions/10330002/sum-of-small-double-numbers-c
 * see https://en.wikipedia.org/wiki/Kahan_summation_algorithm
 */
struct compensated_summation_t {
	double sum = 0;
	double correction = 0;
};

template <typename T>
compensated_summation_t compensated_summation(const compensated_summation_t& accumulation, T value)
{
	compensated_summation_t result;

	double y = (double)value - accumulation.correction;
	double t = accumulation.sum + y;
	result.correction = (t - accumulation.sum) - y;
	result.sum = t;

	return result;
}
}

} // pvcop::db::primitive

} // pvcop::db

} // pvcop

#endif // __DB_ARRAY_PRIMITIVE_COMPENSATED_SUMMATION_H__
