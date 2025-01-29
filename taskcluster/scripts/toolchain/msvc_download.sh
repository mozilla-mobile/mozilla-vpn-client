#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

python3 taskcluster/scripts/vsdownload.py --accept-license --dest msvc \
Microsoft.VisualStudio.Component.VC.Redist.14.Latest \
Microsoft.VisualCpp.Tools.HostX64.TargetARM64 \
Microsoft.VisualCpp.Tools.HostX64.TargetX64 \
Microsoft.VisualStudio.Component.VC.14.42.17.12.Tools.x86.x64 \
Microsoft.VisualStudio.Component.VC.14.42.17.12.Tools.ARM64 \
Microsoft.VisualStudio.Component.VC.14.42.17.12.ATL.ARM64 \
Microsoft.VisualStudio.Component.VC.14.42.17.12.ATL \
Microsoft.VisualStudio.Component.VC.Redist.MSM \
Microsoft.VisualStudio.Component.VC.CoreBuildTools \
Microsoft.VisualStudio.Component.Windows10SDK.19041 \ \
Microsoft.VisualStudio.VC.MSBuild.Llvm

cp taskcluster/scripts/fetch/enter_dev_shell.ps1 msvc/

tar -cJf $UPLOAD_DIR/msvc.tar.xz msvc/
