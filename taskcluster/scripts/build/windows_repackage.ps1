# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

Set-Location $env:TASK_WORKDIR
Get-ChildItem env:

# Add CMake to the path
$CMAKE_INSTALL_DIR = (resolve-path "$env:MOZ_FETCHES_DIR\cmake-*-windows-x86_64")
$env:PATH += ";$CMAKE_INSTALL_DIR\bin"

# Download ninja
Invoke-WebRequest -UseBasicParsing -Uri https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-win.zip -OutFile $env:MOZ_FETCHES_DIR\ninja-win.zip
$NINJA_FILE_HASH = (Get-FileHash "$env:MOZ_FETCHES_DIR\ninja-win.zip").Hash
if($NINJA_FILE_HASH -ne "07fc8261b42b20e71d1720b39068c2e14ffcee6396b76fb7a795fb460b78dc65") {
    throw "Invalid file hash: $NINJA_FILE_HASH"
}
Expand-Archive -Path "$env:MOZ_FETCHES_DIR\ninja-win.zip" -DestinationPath "$CMAKE_INSTALL_DIR\bin"

# Install the wix toolset
msiexec /i "$env:MOZ_FETCHES_DIR/wix-cli-x64.msi"

# Extract the build artifacts
Write-Output "Extracting: $env:MOZ_FETCHES_DIR\unsigned.zip"
Expand-Archive -Path "$env:MOZ_FETCHES_DIR\unsigned.zip" -DestinationPath "$env:TASK_WORKDIR\unsigned"

# Package them with CMake and Wix
cmake -B "$env:TASK_WORKDIR\unsigned" -S "$env:VCS_PATH\windows\installer" -GNinja
cmake --build "$env:TASK_WORKDIR\unsigned" --target msi

# Upload the MSI installer
New-Item -ItemType Directory -Path "$env:TASK_WORKDIR\artifacts" -Force
Move-Item -Path "$env:TASK_WORKDIR\unsigned\MozillaVPN.msi" -Destination "$env:TASK_WORKDIR\artifacts"
