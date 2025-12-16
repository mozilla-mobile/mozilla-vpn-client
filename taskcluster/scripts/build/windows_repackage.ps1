# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

Set-Location $env:TASK_WORKDIR
Get-ChildItem env:

# Install the wix toolset
msiexec /i "$env:MOZ_FETCHES_DIR/wix-cli-x64.msi"

# Extract the build artifacts
Write-Output "Extracting: $env:MOZ_FETCHES_DIR\unsigned.zip"
Expand-Archive -Path "$env:MOZ_FETCHES_DIR\unsigned.zip" -DestinationPath "$env:TASK_WORKDIR\unsigned"

# Package them with CMake and Wix
# TODO: We might have to install CMake?
cmake -B "$env:TASK_WORKDIR\unsigned" -S "$env:VCS_PATH\windows\installer"
cmake --build "$env:TASK_WORKDIR\unsigned" --target msi

# Upload the MSI installer
New-Item -ItemType Directory -Path "$env:TASK_WORKDIR\artifacts" -Force
Move-Item -Path "$env:TASK_WORKDIR\unsigned\MozillaVPN.msi" -Destination "$env:TASK_WORKDIR\artifacts"
