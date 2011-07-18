# -*- buffer-read-only: t -*- vi: set ro:
# DO NOT EDIT! GENERATED AUTOMATICALLY!
# mkstemps.m4 serial 1
dnl Copyright (C) 2009-2011 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([gl_FUNC_MKSTEMPS],
[
  AC_REQUIRE([gl_STDLIB_H_DEFAULTS])

  dnl Persuade glibc <stdlib.h> to declare mkstemps().
  AC_REQUIRE([AC_USE_SYSTEM_EXTENSIONS])

  AC_CHECK_FUNCS_ONCE([mkstemps])
  if test $ac_cv_func_mkstemps != yes; then
    HAVE_MKSTEMPS=0
    AC_LIBOBJ([mkstemps])
  fi
])
