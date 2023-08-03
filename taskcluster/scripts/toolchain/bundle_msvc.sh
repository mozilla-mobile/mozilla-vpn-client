#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script creates a qt-bundle that we can use in xcode-cloud and 
# in the taskcluser/ios builds ( to be coming ... )


git clone https://github.com/mstorsjo/msvc-wine /opt/msvc

cd /opt/msvc
export PYTHONUNBUFFERED=1 
./vsdownload.py --accept-license --major=$MSVC_MAJOR_VERSION --dest /opt/msvc

zip -qr $UPLOAD_DIR/msvc_bundled.zip /opt/msvc/*

