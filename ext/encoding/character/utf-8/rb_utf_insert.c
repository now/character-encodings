/*
 * contents: UTF8.insert module function.
 *
 * Copyright © 2006 Nikolai Weibull <now@bitwi.se>
 */

#include "rb_includes.h"

VALUE
rb_utf_insert(UNUSED(VALUE self), VALUE str, VALUE index, VALUE other)
{
        long offset = NUM2LONG(index);

        StringValue(str);

        long u_len = utf_length(RSTRING(str)->ptr);

        if (abs(offset) > u_len) {
                if (offset < 0)
                        offset -= u_len;
                rb_raise(rb_eIndexError, "index %ld out of string", offset);
        }

        long byte_index;

        if (offset == -1) {
                byte_index = RSTRING(str)->len;
        } else {
                if (offset < 0)
                        offset++;

                char *s = RSTRING(str)->ptr;

                if (offset < 0)
                        s += RSTRING(str)->len;
                byte_index = utf_offset_to_pointer(s, offset) - s;
        }

        rb_str_update(str, byte_index, 0, other);

        return str;
}
