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

utest_dependencies() {
  #brew install oath-toolkit || die
  echo "Nothing to do"
}

utest_qmake_unit() {
  _qmake tests/unit/unit.pro tests.vcxproj || die
}

utest_qmake_auth() {
  echo "TODO VPN-1802: Auth tests are not supported on windows (no oath-toolkit installed, yet?)"
  #_qmake tests/auth/auth.pro tests.vcxproj || die
}

utest_qmake_nativemessaging() {
  _qmake tests/nativemessaging/nativemessaging.pro tests.vcxproj || die
  _qmake extension/app/app.pro mozillavpnnp.vcxproj || die
}

utest_compile_unit() {
  _compile tests.vcxproj tests.exe || die
}

utest_compile_auth() {
  echo "TODO VPN-1802: Auth tests are not supported on windows (no oath-toolkit installed, yet?)"
  #_compile tests.vcxproj tests.exe || die
}

utest_compile_nativemessaging() {
  _compile tests.vcxproj tests.exe || die
  _compile mozillavpnnp.vcxproj mozillavpnnp.exe || die
}

utest_run_unit() {
  ./tests.exe || die
}

utest_run_auth() {
  echo "TODO VPN-1802: Auth tests are not supported on windows (no oath-toolkit installed, yet?)"
  #./tests.exe || die
}

utest_run_nativemessaging() {
  echo "TODO VPN-1801: No native-messaging tests for windows yet!"
  #./tests.exe ./mozillavpnnp.exe || die
}

utest_cleanup_unit() {
  _cleanup tests.vcxproj tests.exe || die
}

utest_cleanup_auth() {
  echo "TODO VPN-1802: Auth tests are not supported on windows (no oath-toolkit installed, yet?)"
  #_cleanup tests.vcxproj tests.exe || die
}

utest_cleanup_nativemessaging() {
  _cleanup mozillavpnnp.vcxproj mozillavpnnp.exe || die
  _cleanup tests.vcxproj tests.exe || die
}

## Lottie tests

lottie_qmake_unit() {
  _qmake lottie/tests/unit/unit.pro lottie_tests.vcxproj || die
}

lottie_qmake_qml() {
  _qmake lottie/tests/qml/qml.pro tst_lottie.vcxproj || die
}

lottie_compile_unit() {
  _compile lottie_tests.vcxproj lottie_tests.exe || die
}

lottie_compile_qml() {
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

qmltest_qmake() {
  _qmake tests/qml/qml.pro qml_tests.vcxproj || die
}

qmltest_compile() {
  _compile qml_tests.vcxproj qml_tests.exe || die
}

qmltest_run() {
  ./qml_tests.exe || die
}

qmltest_cleanup() {
  _cleanup qml_tests.vcxproj qml_tests.exe || die
}
