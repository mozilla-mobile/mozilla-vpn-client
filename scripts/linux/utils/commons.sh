#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Internal methods

_utest_cleanup() {
  (cd "$1" && make distclean) || echo "No make distclean error reported"
  (cd "$1" && rm -rf .qm .moc .obj .rcc) || die
}

_utest_qmake() {
  (
    cd "$1" || die
    [ -f Makefile ] && die "Unexpected Makefile"

    qmake \
      CONFIG+=DUMMY \
      CONFIG+=debug \
      QMAKE_CXXFLAGS+=--coverage QMAKE_LFLAGS+=--coverage \
      QT+=svg \
      CONFIG+=webextension \
      CONFIG+=AUTHTEST || die "Compilation failed"

    [ -f Makefile ] || die "Expected Makefile"
  )
}

_utest_compile() {
  (cd "$1" && make -j8) || die
}

# Public methods

utest_dependencies() {
  echo "Nothing to do here"
}

utest_qmake_unit() {
  _utest_qmake tests/unit || die
}

utest_qmake_auth() {
  _utest_qmake tests/auth || die
}

utest_qmake_nativemessaging() {
  _utest_qmake tests/nativemessaging || die
  _utest_qmake extension/app || die
}

utest_compile_unit() {
  _utest_compile tests/unit || die
}

utest_compile_auth() {
  _utest_compile tests/auth || die
}

utest_compile_nativemessaging() {
  _utest_compile tests/nativemessaging || die
  _utest_compile extension/app || die
}

utest_run_unit() {
  ./tests/unit/tests || die
}

utest_run_auth() {
  ./tests/auth/tests || die
}

utest_run_nativemessaging() {
  ./tests/nativemessaging/tests ./extension/app/mozillavpnnp || die
}

utest_cleanup_unit() {
  _utest_cleanup tests/unit
}

utest_cleanup_auth() {
  _utest_cleanup tests/auth
}

utest_cleanup_nativemessaging() {
  _utest_cleanup extension/app
  _utest_cleanup tests/nativemessaging
}
