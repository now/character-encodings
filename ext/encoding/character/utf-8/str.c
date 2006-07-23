/*
 * contents: String functions.
 *
 * Copyright © 2005 Nikolai Weibull <work@rawuncut.elitemail.org>
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "str.h"


/* {{{1
 * Retrieve the offset/index of ‘needle’ in ‘haystack’ which is of size
 * ‘haystack_len’.
 */
int
strnstr(const char *haystack, const char *needle, size_t haystack_len)
{
	assert(haystack != NULL);
	assert(needle != NULL);

	size_t needle_len = strlen(needle);

	if (needle_len == 0)
		return 0;

	if (haystack_len < needle_len)
		return -1;

	const char *end = haystack + haystack_len - needle_len;
	for (const char *p = haystack; *p != '\0' && p <= end; p++) {
		size_t i;

		for (i = 0; i < needle_len; i++) {
			if (p[i] != needle[i])
				break;
		}

		if (i == needle_len)
			return p - haystack;
	}

	return -1;
}


/* {{{1
 * Retrieve the index/offset of the right-most occurence of ‘needle’ in
 * ‘haystack’, or -1 if it doesn't exist.
 */
int
strrstr(const char *haystack, const char *needle)
{
	assert(haystack != NULL);
	assert(needle != NULL);

	size_t needle_len = strlen(needle);
	size_t haystack_len = strlen(haystack);

	if (needle_len == 0)
		return haystack_len;

	if (haystack_len < needle_len)
		return -1;

	for (const char *p = haystack + haystack_len - needle_len; p >= haystack; p--) {
		size_t i;

		for (i = 0; i < needle_len; i++) {
			if (p[i] != needle[i])
				break;
		}

		if (i == needle_len)
			return p - haystack;
	}

	return -1;
}


/* {{{1
 * Retrieve the index/offset of the right-most occurence of ‘needle’ in
 * ‘haystack’, or -1 if it doesn't exist.
 */
int
strrnstr(const char *haystack, const char *needle, size_t haystack_len)
{
	assert(haystack != NULL);
	assert(needle != NULL);

	size_t needle_len = strlen(needle);
	const char *haystack_max = haystack + haystack_len;
	const char *p = haystack;

	while (p < haystack_max && *p != '\0')
		p++;

	if (p < haystack + needle_len)
		return -1;

	p -= needle_len;

	for ( ; p >= haystack; p--) {
		size_t i;

		for (i = 0; i < needle_len; i++) {
			if (p[i] != needle[i])
				break;
		}

		if (i == needle_len)
			return p - haystack;
	}

	return -1;
}


/* }}}1 */
