#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Internal methods

_cleanup() {
  rm -rf "$1" "$2" || die
  rm -rf .qm .moc .obj .rcc || die
}

_qmake() {
  [ -f "$2" ] && die "Unexpected $2"

  qmake \
    -tp vc \
    CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release \
    "$1" || die "Compilation failed"

  [ -f "$2" ] || die "Expected $2"
}

_compile() {
  [ -f "$2" ] && die "Unexpected $2 binary"

  msbuild.exe -t:Build -p:Configuration=Release "$1" || die

  [ -f "$2" ] || die "Expected $2 binary"
}

# Public methods

## Unit-tests

utest_compile_unit() {
  _qmake tests/unit/unit.pro tests.vcxproj || die
  _compile tests.vcxproj tests.exe || die
}

utest_compile_auth() {
  _qmake tests/auth/auth.pro tests.vcxproj || die
  _compile tests.vcxproj tests.exe || die
}

utest_compile_nativemessaging() {
  _qmake tests/nativemessaging/nativemessaging.pro tests.vcxproj || die
  _compile tests.vcxproj tests.exe || die

  (cd extension/bridge && cargo build --release) || die
  [ -f extension/bridge/target/release/mozillavpnnp.exe ] || die "Expected extension/bridge/target/release/mozillavpnnp.exe"
}

utest_run_unit() {
  ./tests.exe || die
}

utest_run_auth() {
  ./tests.exe 2>&1 | tee LOGAUTH.txt
  cat LOGAUTH.txt
}

utest_run_nativemessaging() {
  ./tests.exe extension/bridge/target/release/mozillavpnnp.exe || die
}

utest_cleanup_unit() {
  _cleanup tests.vcxproj tests.exe || die
}

utest_cleanup_auth() {
  _cleanup tests.vcxproj tests.exe || die
}

utest_cleanup_nativemessaging() {
  _cleanup mozillavpnnp.vcxproj mozillavpnnp.exe || die
  _cleanup tests.vcxproj tests.exe || die
}

## Lottie tests

lottie_compile_unit() {
  _qmake lottie/tests/unit/unit.pro lottie_tests.vcxproj || die
  _compile lottie_tests.vcxproj lottie_tests.exe || die
}

lottie_compile_qml() {
  _qmake lottie/tests/qml/qml.pro tst_lottie.vcxproj || die
  _compile tst_lottie.vcxproj tst_lottie.exe || die
}

lottie_run_unit() {
  ./lottie_tests.exe || die
}

lottie_run_qml() {
  ./tst_lottie.exe || die
}

lottie_cleanup_unit() {
  _cleanup lottie_tests.vcxproj lottie_tests.exe || die
}

lottie_cleanup_qml() {
  _cleanup tst_lottie.vcxproj tst_lottie.exe || die
}

## QML tests

qmltest_compile() {
  _qmake tests/qml/qml.pro qml_tests.vcxproj || die
  _compile qml_tests.vcxproj qml_tests.exe || die
}

qmltest_run() {
  ./qml_tests.exe || die
}

qmltest_cleanup() {
  _cleanup qml_tests.vcxproj qml_tests.exe || die
}
