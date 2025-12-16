#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e
export XZ_DEFAULTS="-T0"

python3 -m pip install -r ${VCS_PATH}/requirements.txt
python3 -m aqt install-qt -O $(pwd)/qt-install linux desktop ${QT_VERSION} wasm_singlethread -m qtwebsockets qt5compat qtshadertools

mv $(pwd)/qt-install/${QT_VERSION}/wasm_singlethread $(pwd)/qt-wasm
chmod +x $(pwd)/qt-wasm/bin/qt-cmake

mkdir -p ${UPLOAD_DIR}
tar -cJf ${UPLOAD_DIR}/qt-wasm.tar.xz qt-wasm/
