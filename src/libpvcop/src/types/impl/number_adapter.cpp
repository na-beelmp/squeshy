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

#include <pvcop/types/impl/number_adapter.h>

/*****************************************************************************
 * static things related to prefix
 *****************************************************************************/

static const char* __long_si_prefix[] = {"",     "kilo", "mega",  "giga", "tera",
                                         "peta", "exa",  "zetta", "yotta"};
static const char* __short_si_prefix[] = {"", "k", "M", "G", "T", "P", "E", "Z", "Y"};

// in french, there can be accent on 'e'... D'oh!
static const char* __long_bin_prefix[] = {"",     "kibi", "mebi", "gibi", "tebi",
                                          "pebi", "exbi", "zebi", "yobi"};
static const char* __short_bin_prefix[] = {"", "ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi"};

/*****************************************************************************
 * pvcop::types::__impl::number_adapter::number_adapter
 *****************************************************************************/

pvcop::types::__impl::number_adapter::number_adapter()
{
	init("", PREFIX_NONE, false);
}

/*****************************************************************************
 * pvcop::types::__impl::number_adapter::~number_adapter
 *****************************************************************************/

pvcop::types::__impl::number_adapter::~number_adapter()
= default;

/*****************************************************************************
 * pvcop::types::__impl::number_adapter::init
 *****************************************************************************/

void pvcop::types::__impl::number_adapter::init(const char* format,
                                                const prefix_type type,
                                                bool use_short_prefix)
{
	int i = 0;

	while (format[i] != '\0') {
		if (format[i] == '%') {
			if (format[i + 1] == '%') {
				i += 2;
				continue;
			}
		}
		++i;
	}

	_prefix_type = type;

	if (_prefix_type == PREFIX_SI) {
		_prefixes = use_short_prefix ? __short_si_prefix : __long_si_prefix;
	} else if (_prefix_type == PREFIX_BIN) {
		_prefixes = use_short_prefix ? __short_bin_prefix : __long_bin_prefix;
	} else {
		_prefixes = nullptr;
	}
}
