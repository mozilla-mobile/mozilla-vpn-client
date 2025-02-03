#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

python3 $VCS_PATH/taskcluster/scripts/vsdownload.py \
    --major 17 \
    --architecture arm64 \
    --host-arch=arm64 \
    --only-host yes \
    --sdk-version 10.0.19041  \
    --accept-license \
    --dest msvc \
        Microsoft.VisualStudio.Product.BuildTools \
        Microsoft.VisualStudio.Component.VC.CoreBuildTools \
        Microsoft.VisualStudio.Component.VC.Redist.14.Latest \
        Microsoft.VisualStudio.Component.VC.Tools.ARM64

# Remove vctip.exe it no need to send build telemetry to ms :)
find msvc -type f -name "vctip.exe" -exec rm -f {} \;

# Update the "enter dev shell" script to init with host ARM64 and target ARM64
sed -e 's/*TARGET_ARCH/arm64/g' -e 's/*HOST_ARCH/arm64/g' 
      $VCS_PATH/taskcluster/scripts/toolchain/enter_dev_shell.ps1.in > msvc/enter_dev_shell.ps1

tar -cJf $UPLOAD_DIR/msvc.tar.xz msvc/
