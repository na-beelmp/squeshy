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

#include <pvcop/types/ipv4.h>

#if _WIN32
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif

/******************************************************************************
 *
 * pvcop::types::formatter_ipv4::formatter_ipv4
 *
 ******************************************************************************/
pvcop::types::formatter_ipv4::formatter_ipv4(const char* parameters)
    : formatter<uint32_t>(parameters)
{
}

/******************************************************************************
 *
 * pvcop::types::formatter_ipv4::convert_from_string
 *
 ******************************************************************************/
bool pvcop::types::formatter_ipv4::convert_from_string(const char* str,
                                                       reference value,
                                                       bool* /*pass_autodetect*/) const
{
	if (inet_pton(AF_INET, str, &value) == 1) {
		value = ntohl(value); // use the same endianness as the system for proper sort
		return true;
	}
	return false;
}

/******************************************************************************
 *
 * pvcop::types::formatter_ipv4::convert_to_string
 *
 ******************************************************************************/
int pvcop::types::formatter_ipv4::convert_to_string(char* str,
                                                    size_t str_len,
                                                    const type& value) const
{
	type v = htonl(value);
	if (inet_ntop(AF_INET, &v, str, str_len)) {

		return strlen(str);
	}
	return -1;
}
