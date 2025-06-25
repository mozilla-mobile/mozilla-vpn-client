#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

python3 -m pip install --upgrade pip
python3 -m pip install -r ${VCS_PATH}/requirements.txt

aqt install-qt -O $(pwd)/qt-host-tools linux desktop ${QT_VERSION} --archives qtbase

mkdir -p ${UPLOAD_DIR}
tar -cJf ${UPLOAD_DIR}/qt-host-tools.tar.xz qt-host-tools/
