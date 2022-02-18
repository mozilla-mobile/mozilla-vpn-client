#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Internal methods

_utest_cleanup() {
  rm -rf "$1" "$2" || die
  rm -rf .qm .moc .obj .rcc || die
}

_utest_qmake() {
  QMAKE=qmake
  if [[ "$QT_MACOS_BIN" ]]; then
    QT_BIN=$QT_MACOS_BIN
    QMAKE="$QT_BIN/qmake"
    export PATH="$QT_BIN:$PATH"
  fi

  [ -d "$2" ] && die "Unexpected $2 folder"

  $QMAKE \
    -spec macx-xcode \
    QTPLUGIN+=qsvg \
    MVPN_MACOS=1 \
    CONFIG-=debug CONFIG+=release CONFIG-=debug_and_release \
    CONFIG+=sdk_no_version_check \
    CONFIG+=coverage \
    "$1" || die "Compilation failed"

  [ -d "$2" ] || die "Expected $2 folder"
}

_utest_compile() {
  [ -f "$2" ] && die "Unexpected $2 binary"

  xcodebuild build \
  HEADER_SEARCH_PATHS="\$(HEADER_SEARCH_PATHS) $(brew --prefix oath-toolkit)/include" \
    CODE_SIGN_IDENTITY="" \
    CODE_SIGNING_REQUIRED=NO \
    -derivedDataPath=/build \
    -resultBundlePath=/tmp \
    -enableCodeCoverage=YES \
    -project "$1" || die

  [ -f "$2" ] || die "Expected $2 binary"
}

# Public methods

utest_dependencies() {
  brew install oath-toolkit || die
}

utest_qmake_unit() {
  _utest_qmake tests/unit/unit.pro tests.xcodeproj/|| die
}

utest_qmake_auth() {
  _utest_qmake tests/auth/auth.pro tests.xcodeproj/ || die
}

utest_qmake_nativemessaging() {
  _utest_qmake tests/nativemessaging/nativemessaging.pro tests.xcodeproj/ || die
  _utest_qmake extension/app/app.pro mozillavpnnp.xcodeproj/ || die
}

utest_compile_unit() {
  _utest_compile tests.xcodeproj ./Release/tests || die
}

utest_compile_auth() {
  _utest_compile tests.xcodeproj ./Release/tests || die
}

utest_compile_nativemessaging() {
  _utest_compile tests.xcodeproj ./Release/tests || die
  _utest_compile mozillavpnnp.xcodeproj ./Release/mozillavpnnp || die
}

utest_run_unit() {
  ./Release/tests || die
}

utest_run_auth() {
  ./Release/tests || die
}

utest_run_nativemessaging() {
  ./Release/tests ./Release/mozillavpnnp || die
}

utest_cleanup_unit() {
  _utest_cleanup tests.xcodeproj/ ./Release/tests
}

utest_cleanup_auth() {
  _utest_cleanup tests.xcodeproj/ ./Release/tests
}

utest_cleanup_nativemessaging() {
  _utest_cleanup mozillavpnnp.xcodeproj/ ./Release/mozillavpnnp
  _utest_cleanup tests.xcodeproj/ ./Release/tests
}
