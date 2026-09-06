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

#ifndef __PVCOP_DB_IMPL_TYPES_H__
#define __PVCOP_DB_IMPL_TYPES_H__

#include <bitset>

namespace pvcop
{

namespace core
{

/*
 * This casting union is used to safely store invalid values
 * indexes in the strings dictionary
 */
template <typename Torg, typename Tdest>
union cast_t {
	cast_t() {}
	cast_t(Torg o) : as_org(o) {}
	Tdest cast() { return as_dst; }

  private:
	Torg as_org;
	Tdest as_dst;
};

/*
 * Specialization for float/double to make a static_cast
 * instead of a reinterpret_cast
 */
template <typename Torg>
union cast_t<Torg, float> {
	cast_t() {}
	cast_t(Torg o) : as_org((Torg)o) {}
	float cast() { return (float)as_org; }

  private:
	Torg as_org;
};
template <typename Tdest>
union cast_t<float, Tdest> {
	cast_t() {}
	cast_t(float o) : as_dest((Tdest)o) {}
	Tdest cast() { return (Tdest)as_dest; }

  private:
	Tdest as_dest;
};
template <typename Torg>
union cast_t<Torg, double> {
	cast_t() {}
	cast_t(Torg o) : as_org((Torg)o) {}
	double cast() { return (double)as_org; }

  private:
	Torg as_org;
};
template <typename Tdest>
union cast_t<double, Tdest> {
	cast_t() {}
	cast_t(double o) : as_dest((Tdest)o) {}
	Tdest cast() { return (Tdest)as_dest; }

  private:
	Tdest as_dest;
};

template <typename T>
union bitset_cast_t {
	using value_type = std::bitset<sizeof(T) * 8>;

	bitset_cast_t(T v) : as_value(v){};
	value_type cast() { return as_bits; }

  private:
	T as_value;
	value_type as_bits;
};

} // pvcop::db

} // pvcop

#endif // __PVCOP_DB_IMPL_TYPES_H__
