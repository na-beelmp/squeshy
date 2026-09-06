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

#include <pvcop/types/mac_address.h>

union packed_mac_t {
	uint64_t as_int;
	struct {
		uint8_t b1;
		uint8_t b2;
		uint8_t b3;
		uint8_t b4;
		uint8_t b5;
		uint8_t b6;
	};
};

/******************************************************************************
 *
 * pvcop::types::formatter_mac_address::formatter_mac_address
 *
 ******************************************************************************/
pvcop::types::formatter_mac_address::formatter_mac_address(const char*) : formatter<uint64_t>("")
{
}

/******************************************************************************
 *
 * pvcop::types::formatter_mac_address::convert_from_string
 *
 ******************************************************************************/
bool pvcop::types::formatter_mac_address::convert_from_string(const char* str,
                                                              reference value,
                                                              bool* /*pass_autodetect*/) const
{
	const char* format;
	char dummy;

	/* Network manufacturers considers a 0-filled MAC address as an error, so we are.
	 */
	value = 0;

	/* Used sscanf conversion specifiers indicate 2 characters long hexadecimal values which
	 * are saved into unsigned chars. Last char specifier is used to detect any extra trailing
	 * character which makes the MAC address ill-formed.
	 */
	if (str[2] == ':') {
		format = "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx%c";
	} else if (str[2] == '-') {
		format = "%2hhx-%2hhx-%2hhx-%2hhx-%2hhx-%2hhx%c";
	} else if (str[4] == '.') {
		format = "%2hhx%2hhx.%2hhx%2hhx.%2hhx%2hhx%c";
	} else {
		return false;
	}

	packed_mac_t mac = {0};

	if (sscanf(str, format, &mac.b6, &mac.b5, &mac.b4, &mac.b3, &mac.b2, &mac.b1, &dummy) == 6) {
		value = mac.as_int;
		return true;
	}

	return false;
}

/******************************************************************************
 *
 * pvcop::types::formatter_mac_address::convert_to_string
 *
 ******************************************************************************/
int pvcop::types::formatter_mac_address::convert_to_string(char* str,
                                                           size_t str_len,
                                                           const type& value) const
{
	if (str_len < 18) {
		// too short target string (including NUL char)
		return -1;
	}

	const packed_mac_t mac = {value};

	return snprintf(str, str_len, "%02X:%02X:%02X:%02X:%02X:%02X", mac.b6, mac.b5, mac.b4, mac.b3,
	                mac.b2, mac.b1);
}
