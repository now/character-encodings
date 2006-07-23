/*
 * contents: Unicode line-breaking properties.
 *
 * Copyright (C) 2004 Nikolai Weibull <source@pcppopper.org>
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "unicode.h"
#include "data/break.h"


/* Figure out what break type the Unicode character ‘c’ possesses, if any.
 * This information is used for finding word and line boundaries, which is
 * useful when displaying Unicode text on screen. */
static UnicodeBreakType
break_type(const int16_t table[], unsigned int page, unichar c)
{
        int16_t break_property = table[page];

        return (break_property >= UNICODE_MAX_TABLE_INDEX) ? 
                break_property - UNICODE_MAX_TABLE_INDEX :
                break_property_data[break_property][c & 0xff];
}

UnicodeBreakType
unichar_break_type(unichar c)
{
	if (c <= UNICODE_LAST_CHAR_PART1)
                return break_type(break_property_table_part1, c >> 8, c);

        if (c >= UNICODE_FIRST_CHAR_PART2 && c <= UNICODE_LAST_CHAR)
                return break_type(break_property_table_part2,
                                  (c - 0xe0000) >> 8, c);

        return UNICODE_BREAK_UNKNOWN;
}
