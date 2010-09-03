#!/bin/sh

aclocal || exit 1
autoheader || exit 1
libtoolize --automake --copy --force || exit 1
intltoolize --force --copy || exit 1
automake --add-missing --copy --include-deps || exit 1
autoconf || exit 1

if [ -z "$1" ] ; then
  echo
  echo 'FCITX now prepared to build. Run:'
  echo " ./configure && make"
else
  ./configure "$@" || exit 1
  make || exit 1
fi

