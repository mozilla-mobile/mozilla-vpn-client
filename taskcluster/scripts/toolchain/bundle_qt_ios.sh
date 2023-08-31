#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script creates a qt-bundle that we can use in xcode-cloud and 
# in the taskcluser/ios builds ( to be coming ... )

$QT_VERSION=6.4.3 # We currently have build issues on X-Code Cloud, therefore we need to override that. 

python3 -m pip install --upgrade pip
python3 -m pip install --upgrade aqtinstall

aqt install-qt -O qt_ios mac desktop $QT_VERSION -m all
aqt install-qt -O qt_ios mac ios $QT_VERSION -m all

zip -qr $UPLOAD_DIR/qt6_ios.zip qt_ios/*

