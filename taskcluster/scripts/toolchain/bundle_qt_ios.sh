#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script creates a qt-bundle that we can use in xcode-cloud and 
# in the taskcluser/ios builds ( to be coming ... )

# This script takes one argument - the Qt version.
if [[ $# -ge 1 ]]; then
  QT_VERSION="$1"
else
  echo "No Qt version specified." >&2
  echo "" >&2
  echo "Usage: $0 <QT_VERSION>" >&2
  exit 1
fi

python3 -m pip install --upgrade pip
python3 -m pip install --upgrade aqtinstall

aqt install-qt -O qt_ios mac desktop $QT_VERSION -m qtwebsockets qtnetworkauth
aqt install-qt -O qt_ios mac ios $QT_VERSION -m qtwebsockets qtnetworkauth

zip -qr $UPLOAD_DIR/qt6_ios.zip qt_ios/*

