#ifndef _FURL_CHAR_H
#define _FURL_CHAR_H

#include <stdint.h>

#ifdef FURL_UTF16_CHAR
typedef uint16_t char_t;
#define TO_ASCII(c) ((c) & 0x00FF)
#else
typedef char char_t;
#define TO_ASCII(c) ((c))
#endif

#endif
