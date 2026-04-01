# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

Set-Location $env:TASK_WORKDIR
Get-ChildItem env:

# Add CMake to the path
$CMAKE_INSTALL_DIR = (resolve-path "$env:MOZ_FETCHES_DIR\cmake-*-windows-x86_64")
$env:PATH += ";$CMAKE_INSTALL_DIR\bin"

# Install the wix toolset
New-Item -ItemType Directory -Path "$env:TASK_WORKDIR\wix" -Force
Copy-Item -Path (Resolve-Path "$env:MOZ_FETCHES_DIR\wix.*.nupkg") -Destination $env:TASK_WORKDIR\wix\wix.zip
Expand-Archive -Path "$env:TASK_WORKDIR\wix\wix.zip" -DestinationPath "$env:TASK_WORKDIR\wix"
$WIX_TOOLS_PATH = (Resolve-Path "$env:TASK_WORKDIR\wix\tools\net*")
$env:PATH += ";$WIX_TOOLS_PATH\any"

# Extract the build artifacts
Write-Output "Extracting: $env:MOZ_FETCHES_DIR\unsigned.zip"
Expand-Archive -Path "$env:MOZ_FETCHES_DIR\unsigned.zip" -DestinationPath "$env:TASK_WORKDIR\unsigned"

# Package them with CMake and Wix
cmake -B "$env:TASK_WORKDIR\unsigned" -S "$env:VCS_PATH\windows\installer" -GNinja -DCMAKE_MAKE_PROGRAM="$env:MOZ_FETCHES_DIR\ninja.exe"
cmake --build "$env:TASK_WORKDIR\unsigned" --target msi

# Upload the MSI installer
New-Item -ItemType Directory -Path "$env:TASK_WORKDIR\artifacts" -Force
cmake --install "$env:TASK_WORKDIR\unsigned" --prefix "$env:TASK_WORKDIR\artifacts" --component msi
