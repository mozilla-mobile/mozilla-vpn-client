#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set -e

sudo apt-get update
sudo apt-get -y install python3-pip icu-devtools

python3 -m pip install -r ${VCS_PATH}/requirements.txt
python3 -m aqt install-qt -O $(pwd)/qt-install linux desktop ${QT_VERSION} --archives qtbase qtdeclarative qttools

mv $(pwd)/qt-install/${QT_VERSION}/gcc_64 $(pwd)/qt-host-tools
find $(pwd)/qt-host-tools/lib -name '*.a' -delete

# Copy the libicu libraries into Qt's library path.
DEB_HOST_MULTIARCH=$(dpkg-architecture -q DEB_HOST_MULTIARCH)
for ICULIB in $(find /usr/lib/${DEB_HOST_MULTIARCH} -type f -name 'libicu*.so.*'); do
    FILENAME=$(basename ${ICULIB})
    LINKNAME=$(echo ${FILENAME} | awk -F. '{print $1 "." $2 "." $3}')
    cp ${ICULIB} $(pwd)/qt-host-tools/lib/
    (cd $(pwd)/qt-host-tools/lib && ln -s ${FILENAME} ${LINKNAME})
done

mkdir -p ${UPLOAD_DIR}
tar -cJf ${UPLOAD_DIR}/qt-host-tools.tar.xz qt-host-tools/
