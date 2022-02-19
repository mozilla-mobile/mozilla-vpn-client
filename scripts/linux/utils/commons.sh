#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Internal methods

_cleanup() {
  (cd "$1" && make distclean) || echo "No make distclean error reported"
  (cd "$1" && rm -rf .qm .moc .obj .rcc) || die
}

_qmake() {
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

_compile() {
  (cd "$1" && make -j8) || die
}

# Public methods

## Unit tests

utest_dependencies() {
  echo "Nothing to do here"
}

utest_qmake_unit() {
  _qmake tests/unit || die
}

utest_qmake_auth() {
  _qmake tests/auth || die
}

utest_qmake_nativemessaging() {
  _qmake tests/nativemessaging || die
  _qmake extension/app || die
}

utest_compile_unit() {
  _compile tests/unit || die
}

utest_compile_auth() {
  _compile tests/auth || die
}

utest_compile_nativemessaging() {
  _compile tests/nativemessaging || die
  _compile extension/app || die
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
  _cleanup tests/unit
}

utest_cleanup_auth() {
  _cleanup tests/auth
}

utest_cleanup_nativemessaging() {
  _cleanup extension/app
  _cleanup tests/nativemessaging
}

## Lottie tests

lottie_qmake_unit() {
  _qmake lottie/tests/unit || die
}

lottie_qmake_qml() {
  _qmake lottie/tests/qml || die
}

lottie_compile_unit() {
  _compile lottie/tests/unit || die
}

lottie_compile_qml() {
  _compile lottie/tests/qml || die
}

lottie_run_unit() {
  ./lottie/tests/unit/lottie_tests || die
}

lottie_run_qml() {
  ./lottie/tests/qml/tst_lottie -platform offscreen || die
}

lottie_cleanup_unit() {
  _cleanup lottie/tests/unit || die
}

lottie_cleanup_qml() {
  _cleanup lottie/tests/qml || die
}

## QML tests

qmltest_qmake() {
  _qmake tests/qml || die
}

qmltest_compile() {
  _compile tests/qml || die
}

qmltest_run() {
  ./tests/qml/qml_tests -platform offscreen|| die
}

qmltest_cleanup() {
  _cleanup tests/qml || die
}
