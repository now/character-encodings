/*
 * contents: UTF8.length module function.
 *
 * Copyright © 2006 Nikolai Weibull <now@bitwi.se>
 */

#include "rb_includes.h"

VALUE
rb_utf_length(UNUSED(VALUE self), VALUE str)
{
        return UINT2NUM(utf_length(StringValuePtr(str)));
}
