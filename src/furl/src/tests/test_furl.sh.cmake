#!/bin/bash
#
# DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
# Version 2, December 2004
#
# Copyright (C) 2012 Adrien Guinet <adrien@guinet.me>
#
# Everyone is permitted to copy and distribute verbatim or modified
# copies of this license document, and changing it is allowed as long
# as the name is changed.
#
# DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
# TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
#
#  0. You just DO WHAT THE FUCK YOU WANT TO.

# Test furl output
# furl output format is :
# scheme,credential,domain,tld,port,resource_path,query_string,fragment
#
# To add an URL to the list of URL to test, simply use the provided add_url.sh script:
# $ ./add_url.sh http://www.test.com/myweirdurl
# It will basically add http://www.test.com/myweirdurl to urls.txt, and the output of furl
# to urls.txt.ref

# Get script absolute directory
SCRIPTDIR="@FURL_TOOLS_DIR@"
URLS="@FURL_TESTS_SRC_DIR@/urls.txt"
URLS_REF="@FURL_TESTS_SRC_DIR@/urls.txt.ref"
URLS_CMP="@FURL_TESTS_DIR@/urls.txt.cmp"

# Execute furl-* on urls.txt and compare to the reference output

# UTF-8
"$SCRIPTDIR/furl-utf8" < "$URLS" > "$URLS_CMP" || exit $1
diff -u "$URLS_REF" "$URLS_CMP"
RET=$?
if [ $RET -eq 0 ]
then
	rm "$URLS_CMP"
fi
GRET=$RET

# UTF-16
iconv -f UTF8 -t UTF16LE < "$URLS" | "$SCRIPTDIR/furl-utf16" | iconv -f UTF16LE -t UTF8 > "$URLS_CMP" || exit $1
diff -u "$URLS_REF" "$URLS_CMP"
RET=$?
if [ $RET -eq 0 ]
then
	rm "$URLS_CMP"
fi
GRET=$RET

exit $GRET
