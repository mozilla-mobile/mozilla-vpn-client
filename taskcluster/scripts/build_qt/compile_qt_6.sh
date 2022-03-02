#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


mkdir qt_dist
mkdir artifacts

echo "Download QT 6.2.3"
curl -o qt.zip -L https://download.qt.io/archive/qt/6.2/6.2.3/single/qt-everywhere-src-6.2.3.zip
unzip -a qt.zip || true 
echo "Building QT"
./scripts/utils/qt6_compile.sh qt-everywhere-src-6.2.3 qt_dist 
echo "Build Qt- Creating dist artifact"
zip -r artifacts/qt6_mac.zip qt_dist