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
      CONFIG+=AUTHTEST || die "Compilation failed"

    [ -f Makefile ] || die "Expected Makefile"
  )
}

_compile() {
  (cd "$1" && make -j8) || die
}

_grcov() {
  (
    cd "$1" || die
    grcov .obj  -s . -t lcov --branch --ignore-not-existing || die "Failed to run grcov"
  ) > "$2"
  [ -f "$2" ] || die "Expected $2 grcov output"
}

# Public methods

## Unit tests

utest_compile_unit() {
  _qmake tests/unit || die
  _compile tests/unit || die
}

utest_compile_auth() {
  _qmake tests/auth || die
  _compile tests/auth || die
}

utest_compile_nativemessaging() {
  _qmake tests/nativemessaging || die
  _compile tests/nativemessaging || die
  [ -f extension/bridge/target/release/mozillavpnnp ] || die "Expected extension/bridge/target/release/mozillavpnnp"
}

utest_run_unit() {
  ./tests/unit/tests || die
}

utest_run_auth() {
  ./tests/auth/tests || die
}

utest_run_nativemessaging() {
  ./tests/nativemessaging/tests ./extension/bridge/target/release/mozillavpnnp || die
}

utest_cleanup_unit() {
  _cleanup tests/unit
}

utest_cleanup_auth() {
  _cleanup tests/auth
}

utest_cleanup_nativemessaging() {
  _cleanup tests/nativemessaging
}

utest_grcov_unit() {
  _grcov ./tests/unit/ "$1"
}

utest_grcov_auth() {
  _grcov ./tests/auth/ "$1"
}

utest_grcov_nativemessaging() {
  _grcov ./tests/nativemessaging/ "$1"
}

## Lottie tests

lottie_compile_unit() {
  _qmake lottie/tests/unit || die
  _compile lottie/tests/unit || die
}

lottie_compile_qml() {
  _qmake lottie/tests/qml || die
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

lottie_grcov_unit() {
  _grcov ./lottie/tests/unit "$1" || die
}

lottie_grcov_qml() {
  _grcov ./lottie/tests/qml "$1" || die
}

## QML tests

qmltest_compile() {
  _qmake tests/qml || die
  _compile tests/qml || die
}

qmltest_run() {
  ./tests/qml/qml_tests -platform offscreen|| die
}

qmltest_cleanup() {
  _cleanup tests/qml || die
}

qmltest_grcov() {
  _grcov ./tests/qml "$1"
}
