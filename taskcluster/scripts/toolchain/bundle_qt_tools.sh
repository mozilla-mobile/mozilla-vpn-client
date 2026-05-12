#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e
export XZ_DEFAULTS="-T0"

# This script takes one argument - the Qt version.
if [[ $# -ge 1 ]]; then
  QT_VERSION="$1"
else
  echo "No Qt version specified." >&2
  echo "" >&2
  echo "Usage: $0 <QT_VERSION>" >&2
  exit 1
fi

python3 -m pip install -r ${VCS_PATH}/requirements.txt
python3 -m aqt install-qt -O $(pwd)/qt-install linux desktop ${QT_VERSION} --archives icu qtbase qtdeclarative qttools --modules qtshadertools

mv $(pwd)/qt-install/${QT_VERSION}/gcc_64 $(pwd)/qt-host-tools
find $(pwd)/qt-host-tools/lib -name '*.a' -delete

mkdir -p ${UPLOAD_DIR}
tar -cJf ${UPLOAD_DIR}/qt-host-tools.tar.xz qt-host-tools/
