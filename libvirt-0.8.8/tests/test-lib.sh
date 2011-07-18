# source this file; set up for tests

test -z "$abs_srcdir" && abs_srcdir=$(pwd)
test -z "$abs_builddir" && abs_builddir=$(pwd)
test -z "$abs_top_srcdir" && abs_top_srcdir=$(pwd)/..
test -z "$abs_top_builddir" && abs_top_builddir=$(pwd)/..
test -z "$LC_ALL" && LC_ALL=C

# Skip this test if the shell lacks support for functions.
unset function_test
eval 'function_test() { return 11; }; function_test'
if test $? != 11; then
  echo "$0: /bin/sh lacks support for functions; skipping this test." 1>&2
  (exit 77); exit 77
fi

test_intro()
{
  name=$1
  if test "$verbose" = "0" ; then
    echo "TEST: $name"
    printf "      "
  fi
}

test_result()
{
  counter=$1
  name=$2
  status=$3
  if test "$verbose" = "0" ; then
    mod=`expr \( $counter + 40 - 1 \) % 40`
    if test "$counter" != 1 && test "$mod" = 0 ; then
        printf " %-3d\n" `expr $counter - 1`
        printf "      "
    fi
    if test "$status" = "0" ; then
        printf "."
    else
        printf "!"
    fi
  else
    if test "$status" = "0" ; then
      printf "%3d) %-60s ... OK\n" "$counter" "$name"
    else
      printf "%3d) %-60s ... FAILED\n" "$counter" "$name"
    fi
  fi
}

test_final()
{
  counter=$1
  status=$2

  if test "$verbose" = "0" ; then
    mod=`expr \( $counter + 1 \) % 40`
    if test "$mod" != "0" && test "$mod" != "1" ; then
      for i in `seq $mod 40`
      do
        printf " "
      done
    fi
    if test "$status" = "0" ; then
      printf " %-3d OK\n" $counter
    else
      printf " %-3d FAILED\n" $counter
    fi
  fi
}

skip_test_()
{
  echo "$0: skipping test: $@" 1>&2
  (exit 77); exit 77
}

require_acl_()
{
  getfacl --version < /dev/null > /dev/null 2>&1 \
    && setfacl --version < /dev/null > /dev/null 2>&1 \
      || skip_test_ "This test requires getfacl and setfacl."

  id -u bin > /dev/null 2>&1 \
    || skip_test_ "This test requires a local user named bin."
}

require_ulimit_()
{
  ulimit_works=yes
  # Expect to be able to exec a program in 10MB of virtual memory,
  # but not in 20KB.  I chose "date".  It must not be a shell built-in
  # function, so you can't use echo, printf, true, etc.
  # Of course, in coreutils, I could use $top_builddir/src/true,
  # but this should be able to work for other projects, too.
  ( ulimit -v 10000; date ) > /dev/null 2>&1 || ulimit_works=no
  ( ulimit -v 20;    date ) > /dev/null 2>&1 && ulimit_works=no

  test $ulimit_works = no \
    && skip_test_ "this shell lacks ulimit support"
}

require_readable_root_()
{
  test -r / || skip_test_ "/ is not readable"
}

# Skip the current test if strace is not available or doesn't work.
require_strace_()
{
  strace -V < /dev/null > /dev/null 2>&1 ||
    skip_test_ 'no strace program'

  strace -qe unlink echo > /dev/null 2>&1 ||
    skip_test_ 'strace does not work'
}

require_built_()
{
  skip_=no
  for i in "$@"; do
    case " $built_programs " in
      *" $i "*) ;;
      *) echo "$i: not built" 1>&2; skip_=yes ;;
    esac
  done

  test $skip_ = yes && skip_test_ "required program(s) not built"
}

uid_is_privileged_()
{
  # Make sure id -u succeeds.
  my_uid=$(id -u) \
    || { echo "$0: cannot run \`id -u'" 1>&2; return 1; }

  # Make sure it gives valid output.
  case $my_uid in
    0) ;;
    *[!0-9]*)
      echo "$0: invalid output (\`$my_uid') from \`id -u'" 1>&2
      return 1 ;;
    *) return 1 ;;
  esac
}

skip_if_()
{
  case $1 in
    root) skip_test_ must be run as root ;;
    non-root) skip_test_ must be run as non-root ;;
    *) ;;  # FIXME?
  esac
}

require_selinux_()
{
  case `ls -Zd .` in
    '? .'|'unlabeled .')
      skip_test_ "this system (or maybe just" \
        "the current file system) lacks SELinux support"
    ;;
  esac
}

very_expensive_()
{
  if test "$RUN_VERY_EXPENSIVE_TESTS" != yes; then
    skip_test_ '
This test is very expensive, so it is disabled by default.
To run it anyway, rerun make check with the RUN_VERY_EXPENSIVE_TESTS
environment variable set to yes.  E.g.,

  env RUN_VERY_EXPENSIVE_TESTS=yes make check
'
  fi
}

require_root_() { uid_is_privileged_ || skip_test_ "must be run as root"; }
skip_if_root_() { uid_is_privileged_ && skip_test_ "must be run as non-root"; }
error_() { echo "$0: $@" 1>&2; (exit 1); exit 1; }
framework_failure() { error_ 'failure in testing framework'; }

mkfifo_or_skip_()
{
  test $# = 1 || framework_failure
  if ! mkfifo "$1"; then
    # Make an exception of this case -- usually we interpret framework-creation
    # failure as a test failure.  However, in this case, when running on a SunOS
    # system using a disk NFS mounted from OpenBSD, the above fails like this:
    # mkfifo: cannot make fifo `fifo-10558': Not owner
    skip_test_ 'NOTICE: unable to create test prerequisites'
  fi
}

test_dir_=$(pwd)

this_test_() { echo "./$0" | sed 's,.*/,,'; }
this_test=$(this_test_)

verbose=0
if test -n "$VIR_TEST_DEBUG" || test -n "$VIR_TEST_VERBOSE" ; then
  verbose=1
fi

# This is a stub function that is run upon trap (upon regular exit and
# interrupt).  Override it with a per-test function, e.g., to unmount
# a partition, or to undo any other global state changes.
cleanup_() { :; }

mktempd="$abs_top_srcdir/build-aux/mktempd"
t_=$("$SHELL" "$mktempd" "$test_dir_" lv-$this_test.XXXXXXXXXX) \
    || error_ "failed to create temporary directory in $test_dir_"

# Run each test from within a temporary sub-directory named after the
# test itself, and arrange to remove it upon exception or normal exit.
trap 'st=$?; cleanup_; d='"$t_"';
    cd '"$test_dir_"' && chmod -R u+rwx "$d" && rm -rf "$d" && exit $st' 0
trap '(exit $?); exit $?' 1 2 13 15

cd "$t_" || error_ "failed to cd to $t_"

if ( diff --version < /dev/null 2>&1 | grep GNU ) 2>&1 > /dev/null; then
  compare() { diff -u "$@"; }
elif ( cmp --version < /dev/null 2>&1 | grep GNU ) 2>&1 > /dev/null; then
  compare() { cmp -s "$@"; }
else
  compare() { cmp "$@"; }
fi

# Local Variables:
# indent-tabs-mode: nil
# End:
