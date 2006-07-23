/*
 * contents: Unicode decomposition handling.
 *
 * Copyright (C) 2004 Nikolai Weibull <source@pcppopper.org>
 */

#include <ruby.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "unicode.h"
#include "private.h"
#include "data/decompose.h"
#include "data/compose.h"


/* {{{1
 * Macros for accessing the combining class property tables for a given
 * character.
 *
 * TODO: Turn these macros into full-fledged functions, as this is rather silly
 * when we have ‹inline› in C99.
 */
#define CC_PART1(page, char) \
        ((combining_class_table_part1[page] >= UNICODE_MAX_TABLE_INDEX) \
         ? (combining_class_table_part1[page] - UNICODE_MAX_TABLE_INDEX) \
         : (cclass_data[combining_class_table_part1[page]][char]))

#define CC_PART2(page, char) \
        ((combining_class_table_part2[page] >= UNICODE_MAX_TABLE_INDEX) \
         ? (combining_class_table_part2[page] - UNICODE_MAX_TABLE_INDEX) \
         : (cclass_data[combining_class_table_part2[page]][char]))

#define COMBINING_CLASS(char) \
        (((char) <= UNICODE_LAST_CHAR_PART1) \
         ? CC_PART1((char) >> 8, (char) & 0xff) \
         : (((char) >= 0xe0000 && (char) <= UNICODE_LAST_CHAR) \
            ? CC_PART2(((char) - 0xe0000) >> 8, (char) & 0xff) \
            : 0))


/* {{{1
 * Hangul syllable [de]composition constants. A lot of work I'd say.
 */
enum {
        SBase = 0xac00,
        LBase = 0x1100,
        VBase = 0x1161,
        TBase = 0x11a7,
        LCount = 19,
        VCount = 21,
        TCount = 28,
        NCount = (VCount * TCount),
        SCount = (LCount * NCount),
        SLast  = (SBase + SCount - 1)
};


/* {{{1
 * Return the combinging class of ‘c’.
 */
inline int
_unichar_combining_class(unichar c)
{
        return COMBINING_CLASS(c);
}


/* {{{1
 * Rearrange ‘str’ so that decomposed characters are arranged according to
 * their combining class.  Do this for at most ‘len’ bytes of data.
 */
static bool
unicode_canonical_ordering_swap(unichar *str, size_t offset, int next)
{
        size_t initial = offset + 1;

        size_t j = initial;
        while (j > 0 && COMBINING_CLASS(str[j - 1]) <= next) {
                unichar c = str[j];
                str[j] = str[j - 1];
                str[j - 1] = c;
                j--;
        }

        return j != initial;
}

static bool
unicode_canonical_ordering_reorder(unichar *str, size_t len)
{
        bool swapped = false;

        int prev = COMBINING_CLASS(str[0]);
        for (size_t i = 0; i < len - 1; i++) {
                int next = COMBINING_CLASS(str[i + 1]);

                if (next != 0 && prev > next)
                        swapped = unicode_canonical_ordering_swap(str, i, next);
                else
                        prev = next;
        }

        return swapped;
}

void
unicode_canonical_ordering(unichar *str, size_t len)
{
        while (unicode_canonical_ordering_reorder(str, len))
                ; /* This loop intentionally left empty. */
}


/* {{{1
 * Decompose the character ‘s’ according to the rules outlined in
 * http://www.unicode.org/unicode/reports/tr15/#Hangul.  ‘r’ should be ‹NULL›
 * or of sufficient length to store the decomposition of ‘s’.  The number of
 * characters stored (or would be if it were non-‹NULL›) in ‘r’ is put in
 * ‘r_len’.
 */
static void
decompose_hangul(unichar s, unichar *r, size_t *r_len)
{
        int SIndex = s - SBase;

        /* If this isn’t a Hangul symbol, then simply return it unchanged. */
        if (SIndex < 0 || SIndex >= SCount) {
                if (r != NULL)
                        r[0] = s;
                *r_len = 1;
                return;
        }

        unichar L = LBase + SIndex / NCount;
        unichar V = VBase + (SIndex % NCount) / TCount;
        unichar T = TBase + SIndex % TCount;

        if (r != NULL) {
                r[0] = L;
                r[1] = V;
        }

        if (T != TBase) {
                if (r != NULL)
                        r[2] = T;
                *r_len = 3;
        } else {
                *r_len = 2;
        }
}


/* {{{1
 * Search the Unicode decomposition table for ‘c’ and depending on the boolean
 * value of ‘compat’, return its compatibility or canonical decomposition if
 * found.
 */
static const char *
get_decomposition(int index, bool compat)
{
        int offset;

        if (compat) {
                offset = decomp_table[index].compat_offset;
                if (offset == UNICODE_NOT_PRESENT_OFFSET)
                        offset = decomp_table[index].canon_offset;
                /* Either .compat_offset or .canon_offset can be missing, but
                 * not both. */
        } else {
                offset = decomp_table[index].canon_offset;
                if (offset == UNICODE_NOT_PRESENT_OFFSET)
                        return NULL;
        }

        return &decomp_expansion_string[offset];
}

static const char *
find_decomposition(unichar c, bool compat)
{
        int begin = 0;
        int end = lengthof(decomp_table);

        if (c < decomp_table[begin].ch || c > decomp_table[end - 1].ch)
                return NULL;

        while (true) {
                int middle = (begin + end) / 2;

                if (c == decomp_table[middle].ch)
                        return get_decomposition(middle, compat);
                else if (middle == begin)
                        break;
                else if (c > decomp_table[middle].ch)
                        begin = middle;
                else
                        end = middle;
        }

        return NULL;
}


/* {{{1
 * Generate the canonical decomposition of ‘c’.  The length of the
 * decomposition is stored in ‘len’.
 */
/* TODO: clean this up. */
unichar *
unicode_canonical_decomposition(unichar c, size_t *len)
{
        const char *decomp;
        unichar *r;

        /* Hangul syllable */
        if (c >= SBase && c <= SLast) {
                decompose_hangul(c, NULL, len);
                r = ALLOC_N(unichar, *len);
                decompose_hangul(c, r, len);
        } else if ((decomp = find_decomposition(c, false)) != NULL) {
                *len = utf_length(decomp);
                r = ALLOC_N(unichar, *len);

                int i;
                const char *p;
                for (p = decomp, i = 0; *p != NUL; p = utf_next(p), i++)
                        r[i] = utf_char(p);
        } else {
                r = ALLOC(unichar);
                *r = c;
                *len = 1;
        }

        /* Supposedly following the Unicode 2.1.9 table means that the
         * decompositions come out in canonical order.  I haven't tested this,
         * but we rely on it here. */
        return r;
}


/* {{{1
 * Combine Hangul characters ‘a’ and ‘b’ if possible, and store the result in
 * ‘result’.  The combinations tried are L,V => LV and LV,T => LVT in that
 * order.
 */
static bool
combine_hangul(unichar a, unichar b, unichar *result)
{
        int LIndex = a - LBase;
        int VIndex = b - VBase;

        if (0 <= LIndex && LIndex < LCount && 0 <= VIndex && VIndex < VCount) {
                *result = SBase + (LIndex * VCount + VIndex) * TCount;
                return true;
        }
        
        int SIndex = a - SBase;
        int TIndex = b - TBase;

        if (0 <= SIndex && SIndex < SCount &&
            (SIndex % TCount) == 0 &&
            0 < TIndex && TIndex < TCount) {
                *result = a + TIndex;
                return true;
        }

        return false;
}


/* {{{1
 * Try to combine the Unicode characters ‘a’ and ‘b’ storing the result in
 * ‘result’.
 */
static uint16_t
compose_index(unichar c)
{
        unsigned int page = c >> 8;

        if (page > COMPOSE_TABLE_LAST)
                return 0;

        /* TODO: why is this signed, exactly? */
        int16_t compose_offset = compose_table[page];
        return (compose_offset >= UNICODE_MAX_TABLE_INDEX) ?
                compose_offset - UNICODE_MAX_TABLE_INDEX :
                compose_data[compose_offset][c & 0xff];
}

static bool
lookup_compose(const uint16_t table[][2], uint16_t index, unichar c,
               unichar *result)
{
        if (c == table[index][0]) {
                *result = table[index][1];
                return true;
        }

        return false;
}

static bool
combine(unichar a, unichar b, unichar *result)
{
        if (combine_hangul(a, b, result))
                return true;

        uint16_t index_a = compose_index(a);
        if (index_a >= COMPOSE_FIRST_SINGLE_START &&
            index_a < COMPOSE_SECOND_START) {
                return lookup_compose(compose_first_single,
                                      index_a - COMPOSE_FIRST_SINGLE_START,
                                      b,
                                      result);
        }

        uint16_t index_b = compose_index(b);
        if (index_b >= COMPOSE_SECOND_SINGLE_START) {
                return lookup_compose(compose_second_single,
                                      index_b - COMPOSE_SECOND_SINGLE_START,
                                      a,
                                      result);
        }

        if (index_a >= COMPOSE_FIRST_START &&
            index_a < COMPOSE_FIRST_SINGLE_START &&
            index_b >= COMPOSE_SECOND_START &&
            index_b < COMPOSE_SECOND_SINGLE_START) {
                unichar r = compose_array[index_a - COMPOSE_FIRST_START][index_b - COMPOSE_SECOND_START];

                if (r != 0) {
                        *result = r;
                        return true;
                }
        }

        return false;
}


/* {{{1
 * Normalize (compose/decompose) characters in ‘str’ so that strings that
 * actually contain the same characters will be recognized as equal for
 * comparison for example.
 */
static size_t
normalize_wc_decompose_one(unichar c, NormalizeMode mode, unichar *buf)
{
        bool do_compat = (mode == NORMALIZE_NFKC || mode == NORMALIZE_NFKD);
        const char *decomp = find_decomposition(c, do_compat);

        if (decomp == NULL) {
                if (buf != NULL)
                        buf[0] = c;
                return 1;
        }

        if (buf != NULL) {
                int i;
                for (i = 0; *decomp != NUL; decomp = utf_next(decomp), i++)
                        buf[i] = utf_char(decomp);
                return i;
        }

        return utf_length(decomp);
}

static void
normalize_wc_decompose(const char *str, size_t max_len, bool use_len,
                       NormalizeMode mode, unichar *buf, size_t *buf_len)
{
        size_t n = 0;
        size_t prev_start = 0;
        const char *end = str + max_len;
        for (const char *p = str; (!use_len || p < end) && *p != NUL; p = utf_next(p)) {
                unichar c = utf_char(p);
                size_t prev_n = n;

                unichar *base = (buf != NULL) ? buf + n : NULL;
                if (c >= SBase && c <= SLast) {
                        size_t len;

                        decompose_hangul(c, base, &len);
                        n += len;
                } else {
                        n += normalize_wc_decompose_one(c, mode, base);
                }

                if (buf != NULL && n > 0 && COMBINING_CLASS(buf[prev_n]) == 0) {
                        unicode_canonical_ordering(buf + prev_start,
                                                   n - prev_start);
                        prev_start = prev_n;
                }
        }

        if (buf != NULL && n > 0)
                unicode_canonical_ordering(buf + prev_start, n - prev_start);

        if (buf != NULL)
                buf[n] = NUL;

        *buf_len = n;
}

unichar *
_utf_normalize_wc(const char *str, size_t max_len, bool use_len, NormalizeMode mode)
{
        size_t n;
        normalize_wc_decompose(str, max_len, use_len, mode, NULL, &n);
        unichar *buf = ALLOC_N(unichar, n + 1);
        normalize_wc_decompose(str, max_len, use_len, mode, buf, &n);

        /* Just return if we don’t want composition. */
        if (!(mode == NORMALIZE_NFC || mode == NORMALIZE_NFKC))
                return buf;

        size_t prev_start = 0;
        int prev_cc = 0;
        for (size_t i = 0; i < n; i++) {
                int cc = COMBINING_CLASS(buf[i]);

                if (i > 0 && (prev_cc == 0 || prev_cc != cc) &&
                    combine(buf[prev_start], buf[i], &buf[prev_start])) {
                        for (size_t j = i + 1; j < n; j++)
                                buf[j - 1] = buf[j];

                        n--;
                        i--;
                        prev_cc = (i == prev_start) ? 0 : COMBINING_CLASS(buf[i - 1]);
                } else {
                        if (cc == 0)
                                prev_start = i;

                        prev_cc = cc;
                }
        }

        buf[n] = NUL;

        return buf;
}


/* {{{1
 * Normalize (compose/decompose) characters in ‘str˚ so that strings that
 * actually contain the same characters will be recognized as equal for
 * comparison for example.
 */
char *
utf_normalize(const char *str, NormalizeMode mode)
{
        unichar *wcs = _utf_normalize_wc(str, 0, false, mode);
        char *utf = ucs4_to_utf8(wcs, NULL, NULL);

        free(wcs);
        return utf;
}


/* {{{1
 * This function is the same as utf_normalize() except that at most ‘len˚
 * bytes are normalized from ‘str’.
 */
char *
utf_normalize_n(const char *str, NormalizeMode mode, size_t len)
{
        unichar *wcs = _utf_normalize_wc(str, len, true, mode);
        char *utf = ucs4_to_utf8(wcs, NULL, NULL);

        free(wcs);
        return utf;
}


/* }}}1 */
