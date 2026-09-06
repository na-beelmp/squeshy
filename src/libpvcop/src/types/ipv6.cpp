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

#include <pvcop/types/ipv6.h>

#include <algorithm>

#if _WIN32
#include <ws2tcpip.h>
#else
#include <arpa/inet.h> // struct in6_addr
#endif

#include <cinttypes> // FIXME: REMOVE

union ipv6_u {
	ipv6_u(in6_addr v)
	{
		as_value = -1; // make sure every bits are set as sizeof(uint128_t) == 24
		as_ipv6 = v;   // but sizeof(in6_addr) == 16
	};
	ipv6_u(pvcop::db::uint128_t v) : as_value(v){};

	in6_addr as_ipv6;
	pvcop::db::uint128_t as_value;
};

static void ntoh128(in6_addr& ipv6)
{
	std::reverse(std::begin(ipv6.s6_addr), std::end(ipv6.s6_addr));
}

static void hton128(in6_addr& ipv6)
{
	ntoh128(ipv6);
}

static uint32_t to_ipv4(const in6_addr& ipv6)
{
	if (IN6_IS_ADDR_V4MAPPED(&ipv6)) {
		return ipv6.s6_addr[12] | ipv6.s6_addr[13] << 8 | ipv6.s6_addr[14] << 16 |
		       ipv6.s6_addr[15] << 24;
	}

	return 0;
}

/******************************************************************************
 *
 * pvcop::types::formatter_ipv6::formatter_ipv6
 *
 ******************************************************************************/
pvcop::types::formatter_ipv6::formatter_ipv6(const char* parameters)
    : formatter<pvcop::db::uint128_t>(parameters)
{
}

/******************************************************************************
 *
 * pvcop::types::formatter_ipv6::convert_from_string
 *
 ******************************************************************************/
bool pvcop::types::formatter_ipv6::convert_from_string(const char* str,
                                                       reference value,
                                                       bool* /*pass_autodetect*/) const
{
	in6_addr ipv6;

	if (inet_pton(AF_INET6, str, &ipv6) == 1) {

		ntoh128(ipv6);
		value = ipv6_u(ipv6).as_value;

		return true;
	} else {
		/**
		 * If conversion from ipv6 string failed, check if we have an ipv4...
		 */
		uint32_t ipv4_value;
		if (inet_pton(AF_INET, str, &ipv4_value) == 1) {
#if __APPLE__
			ipv6.__u6_addr.__u6_addr32[0] = ipv4_value;
			ipv6.__u6_addr.__u6_addr32[1] = 0x0000FFFF;
			ipv6.__u6_addr.__u6_addr32[2] = 0;
			ipv6.__u6_addr.__u6_addr32[3] = 0;
#elif _WIN32
			ipv6 = IN6ADDR_ANY_INIT;
			ipv6.u.Word[2] = htons(0xFFFF);
			memcpy(&ipv6.u.Byte[0], &ipv4_value, 4);
#else
			ipv6.s6_addr32[0] = ipv4_value;
			ipv6.s6_addr32[1] = 0x0000FFFF;
			ipv6.s6_addr32[2] = 0;
			ipv6.s6_addr32[3] = 0;
#endif
			ntoh128(ipv6);
			value = ipv6_u(ipv6).as_value;

			return true;
		}
	}

	return false;
}

/******************************************************************************
 *
 * pvcop::types::formatter_ipv6::convert_to_string
 *
 ******************************************************************************/
int pvcop::types::formatter_ipv6::convert_to_string(char* str,
                                                    size_t str_len,
                                                    const type& value) const
{
	in6_addr ipv6 = ipv6_u(value).as_ipv6;

	if (uint32_t ipv4 = to_ipv4(ipv6)) {
		ipv4 = htonl(ipv4);
		if (inet_ntop(AF_INET, &ipv4, str, str_len)) {
			return strlen(str);
		}
	} else {
		hton128(ipv6);
		if (inet_ntop(AF_INET6, &ipv6, str, str_len)) {
			return strlen(str);
		}
	}

	return -1;
}
