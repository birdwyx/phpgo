dnl $Id$
dnl config.m4 for extension phpgo

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(phpgo, for phpgo support,
[  --with-phpgo             Include phpgo support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(phpgo, whether to enable phpgo support,
[  --enable-phpgo           Enable phpgo support])

if test "$PHP_PHPGO" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-phpgo -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/phpgo.h"  # you most likely want to change this
  dnl if test -r $PHP_PHPGO/$SEARCH_FOR; then # path given as parameter
  dnl   PHPGO_DIR=$PHP_PHPGO
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for phpgo files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PHPGO_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PHPGO_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the phpgo distribution])
  dnl fi

  dnl # --with-phpgo -> add include path
  dnl PHP_ADD_INCLUDE($PHPGO_DIR/include)

  dnl # --with-phpgo -> check for lib and symbol presence
  dnl LIBNAME=phpgo # you may want to change this
  dnl LIBSYMBOL=phpgo # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PHPGO_DIR/$PHP_LIBDIR, PHPGO_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PHPGOLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong phpgo lib version or lib not found])
  dnl ],[
  dnl   -L$PHPGO_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PHPGO_SHARED_LIBADD)

  dnl with split-stack, turn on the following(seems not working)
  dnl CXXFLAGS="-std=c++11 -fsplit-stack $CXXFLAGS"
  dnl otherwise turn on the following:
  CXXFLAGS="-std=c++11 $CXXFLAGS"

  PHP_SUBST(PHPGO_SHARED_LIBADD)
  PHP_REQUIRE_CXX()
  PHP_ADD_LIBRARY(libgo, 1, PHPGO_SHARED_LIBADD)
  dnl PHP_ADD_LIBRARY(libdl, 1, PHPGO_SHARED_LIBADD)
  dnl PHP_ADD_LIBRARY(boost_thread, 1, PHPGO_SHARED_LIBADD)
  PHP_ADD_LIBRARY(stdc++, 1, PHPGO_SHARED_LIBADD)
  PHP_NEW_EXTENSION(phpgo, phpgo.cc go.cc go_chan.cc go_runtime.cc go_timer.cc go_select.cc zend_variables_persist.cc, $ext_shared)
fi
