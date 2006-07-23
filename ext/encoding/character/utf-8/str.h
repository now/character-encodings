/*
 * contents: String functions.
 *
 * Copyright © 2005 Nikolai Weibull <work@rawuncut.elitemail.org>
 */


#ifndef STR_H
#define STR_H


int strnstr(const char *haystack, const char *needle, size_t haystack_len);
int strrstr(const char *haystack, const char *needle);
int strrnstr(const char *haystack, const char *needle, size_t haystack_len);


#endif /* STR_H */
