#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


mkdir qt_dist
mkdir artifacts

wget https://download.qt.io/archive/qt/6.2/6.2.3/single/qt-everywhere-src-6.2.3.zip -o qt.zip
unzip qt.zip -q || true 


./scripts/utils/qt6_compile qt-everywhere-src-6.2.3 qt_dist 

zip -r artifacts/qt6_mac.zip qt_dist