#!/bin/sh

# I think this is executed in a MozillaBuild shell?

set -x
set -e

# Dependencies
python3 -m virtualenv build
build/scripts/pip install -r requirements.txt
python3 scripts/tooltool.py --url http://taskcluster/tooltool.mozilla-releng.net fetch -m windows/vs2017-15.9.tt.manifest


# Install Visual Studio
powershell.exe -ExecutionPolicy Bypass ".\scripts\windows_install_deps.ps1"

cp /c/MozillaVPNBuild/bin/libssl-1_1-x64.dll .
cp /c/MozillaVPNBuild/bin/libcrypto-1_1-x64.dll .
cp /c/MozillaVPNBuild/bin/libEGL.dll .
cp /c/MozillaVPNBuild/bin/libGLESv2.dll .
cp /c/MozillaVPNBuild/*.msm .

# Build
powershell.exe -ExecutionPolicy Bypass ".\scripts\windows_do_in_dev_shell.ps1 ./scripts/windows_compile.bat"

# Package
mkdir unsigned
# copy things to package staging area
cp /c/MozillaVPNBuild/bin/libssl-1_1-x64.dll unsigned
cp /c/MozillaVPNBuild/bin/libcrypto-1_1-x64.dll unsigned
cp /c/MozillaVPNBuild/bin/libEGL.dll unsigned
cp /c/MozillaVPNBuild/bin/libGLESv2.dll unsigned
cp /c/MozillaVPNBuild/*.msm unsigned
cp windows/tunnel/x64/tunnel.dll unsigned
cp windows/tunnel/.deps/wintun/bin/amd64/wintun.dll unsigned
cp balrog/x64/balrog.dll unsigned
cp windows/split-tunnel/mullvad-split-tunnel.cat unsigned
cp windows/split-tunnel/mullvad-split-tunnel.inf unsigned
cp windows/split-tunnel/mullvad-split-tunnel.sys unsigned
cp windows/split-tunnel/WdfCoinstaller01011.dll unsigned
cp extension/app/manifests/windows/mozillavpn.json unsigned
cp *.exe unsigned
# Copy artifact
cd unsigned
zip -r ../build/src/artifacts/unsigned.zip .
