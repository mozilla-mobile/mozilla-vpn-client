#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -x



# This script creates a qt-bundle that we can use in xcode-cloud and 
# in the taskcluser/ios builds ( to be coming ... )

python3 -m pip install --upgrade pip
python3 -m pip install --upgrade aqtinstall

aqt install-qt -O qt_ios mac desktop $QT_VERSION -m qtwebsockets qt5compat
aqt install-qt -O qt_ios mac ios $QT_VERSION -m qtwebsockets qt5compat

zip -qr $UPLOAD_DIR/qt6_ios.zip qt_ios/*

