/*
 * contents: Private Unicode related information.
 *
 * Copyright (C) 2004 Nikolai Weibull <source@pcppopper.org>
 */

#ifndef PRIVATE_H
#define PRIVATE_H

#define NUL '\0'
#define lengthof(ary)   (sizeof(ary) / sizeof((ary)[0]))

#if defined(__GNUC__)
#  define UNUSED(u)   \
        u __attribute__((__unused__))
#  define HIDDEN   \
        __attribute__((visibility("hidden")))
#else
#  define UNUSED(u)   \
        u
#  define HIDDEN(u)
#endif

unichar *_utf_normalize_wc(const char *str, size_t max_len, bool use_len,
                           NormalizeMode mode) HIDDEN;
inline int _unichar_combining_class(unichar c) HIDDEN;

void need_at_least_n_arguments(int argc, int n) HIDDEN;

unichar _utf_char_validated(char const *const str,
                            char const *const str_end) HIDDEN;
char *_utf_offset_to_pointer_validated_impl(const char *str, long offset,
                                            const char *limit, bool noisy) HIDDEN;

char *_utf_offset_to_pointer_validated(const char *str, long offset,
                                       const char *end) HIDDEN;

char *_utf_offset_to_pointer_failable(const char *str, long offset,
                                      const char *end) HIDDEN;

VALUE rb_utf_new(const char *str, long len) HIDDEN;

VALUE rb_utf_new2(const char *str) HIDDEN;

VALUE rb_utf_new5(VALUE obj, const char *str, long len) HIDDEN;

VALUE rb_utf_alloc_using(char *str) HIDDEN;

VALUE rb_utf_dup(VALUE str) HIDDEN;

long rb_utf_index(VALUE str, VALUE sub, long offset) HIDDEN;

bool rb_utf_begin_from_offset(VALUE str, long offset, char **begin,
                              char **limit) HIDDEN;

void rb_utf_begin_from_offset_validated(VALUE str, long offset, char **begin,
                                        char **limit) HIDDEN;

char *rb_utf_prev_validated(const char *begin, const char *p) HIDDEN;

VALUE rb_utf_update(VALUE str, long offset, long len, VALUE replacement) HIDDEN;

char *rb_utf_next_validated(const char *p, const char *end) HIDDEN;

long rb_utf_index_regexp(VALUE str, const char *s, const char *end, VALUE sub,
                         long offset, bool reverse) HIDDEN;

#endif /* PRIVATE_H */
