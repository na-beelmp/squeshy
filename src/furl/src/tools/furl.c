#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include <furl/furl.h>
#include <furl/decode.h>

/* IPv6 tests:

      http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html
      http://[1080:0:0:0:8:800:200C:417A]/index.html
      http://[3ffe:2a00:100:7031::1]
      http://[1080::8:800:200C:417A]/foo
      http://[::192.9.5.5]/ipng
      http://[::FFFF:129.144.52.38]:80/index.html
      http://[2010:836B:4179::836B:4179]
 */

/* readline() - read a line from the file handle.
 * Return an allocated string */
static char_t *furl_readline(FILE *fp)
{
	char_t *str = (char_t*)NULL;
	int len = 256, step = 256, i = 0;

	str = (char_t*)malloc(len * sizeof(char_t));
	if (str == NULL) {
		return str;
	}

	while (1) {
		char_t ch;
		errno = 0;
		fread(&ch, sizeof(char_t), 1, fp);
		if (feof(fp)) {
			str[i] = '\0';
			break;
		}

		if (ch == '\n' || ch == '\r') {
			str[i] = '\0';
			break;
		}
		str[i++] = ch;
		if (i == len - 2) {
			len += step;
			str = (char_t*)realloc(str, len * sizeof(char_t));
			if (str == NULL) {
				return str;
			}
		}
	}

	return str;
}

static size_t furl_strlen(char_t* str)
{
	size_t i = 0;
	while(str[i]) ++i;
	return i;
}

#ifdef FURL_UTF16_CHAR
char_t* to_utf16(const char* str)
{
	const size_t len = strlen(str);
	char_t* ret = (char_t*) malloc((len+1)*sizeof(char_t));
	for (size_t i = 0; i < len; i++) {
		ret[i] = str[i];
	}
	ret[len] = 0;
	return ret;
}
#endif

void print_header()
{
	const char* text = "scheme,credential,subdomain,domain,host,tld,port,resource_path,query_string,fragment\n";
	const size_t len = strlen(text);

#ifdef FURL_UTF16_CHAR
	char_t* text_utf16 = to_utf16(text);
	fwrite(text_utf16, len, sizeof(char_t), stdout);
	free(text_utf16);
#else
	fwrite(text, len, sizeof(char_t), stdout);
#endif
}

int main(int argc, char **argv)
{
	furl_handler_t *fh;
	const char_t newline = '\n';

	fh = furl_init();

	print_header();

	if (isatty(fileno(stdin))) {
		if (argc < 2) {
			fprintf(stderr, "%s url\n", argv[0]);
			exit(1);
		}
#ifdef FURL_UTF16_CHAR
		char_t* url_utf = to_utf16(argv[1]);
		furl_decode(fh, url_utf, strlen(argv[1]));
#else
		furl_decode(fh, argv[1], strlen(argv[1]));
#endif

		furl_show(fh, ',', stdout);
		fwrite(&newline, 1, sizeof(char_t), stdout);

#ifdef FURL_UTF16_CHAR
		free(url_utf);
#endif
	} else {		/* We read from stdin */

		while (1) {
			char_t* strbuf = furl_readline(stdin);

			if (strbuf[0] == '\0') {
				break;
			}

			size_t len = furl_strlen(strbuf);

			if (strbuf[len-1] == '\n') {
				--len;
				if (strbuf[len-1] == '\r') {
					--len;
				}
			}
			strbuf[len] = '\0';

			furl_decode(fh, strbuf, furl_strlen(strbuf));
			furl_show(fh, ',', stdout);
			fwrite(&newline, 1, sizeof(char_t), stdout);
			free(strbuf);
		}
	}

	furl_terminate(fh);

	return 0;
}
