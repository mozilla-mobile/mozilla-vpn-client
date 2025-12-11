# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if (Test-Path Env:TASK_WORKDIR) {
    Set-Location "$env:TASK_WORKDIR"
} else {
    $env:TASK_WORKDIR = Get-Location
    $env:VCS_PATH = Resolve-Path "$PSScriptRoot\..\..\.."
}
Get-ChildItem env:

# Fetch vsdownload.py from the msvc-wine repository.
$MSVC_WIN_COMMIT = "cb78cc0bc91a9e3da69989b76b99d6f44a7d1a69"
$VSDOWNLOAD_URL = "https://github.com/mstorsjo/msvc-wine/raw/$MSVC_WIN_COMMIT/vsdownload.py"
Invoke-WebRequest -Uri $VSDOWNLOAD_URL -OutFile "$env:TASK_WORKDIR\vsdownload.py"

$MSVC_VERSION = "17.8"
$VS_EXTRA_PACKAGES = @(
    'Microsoft.VisualStudio.Component.VC.Redist.MSM'
    'Microsoft.VisualStudio.VC.CMake'
    'Microsoft.VisualStudio.VC.CMake.Project'
)

# Download the Visual Studio SDK
New-Item -ItemType Directory -Path "$env:TASK_WORKDIR\vs2022" -Force
Copy-Item -Path "$PSScriptRoot\enter_dev_shell.ps1" -Destination "$env:TASK_WORKDIR\vs2022"
python $env:TASK_WORKDIR\vsdownload.py `
    --accept-license `
    --skip-recommended `
    --msvc-version $MSVC_VERSION `
    --dest "$env:TASK_WORKDIR\vs2022" `
    @args

# Download additional extensions
python "$env:VCS_PATH\scripts\windows\fetch-vsix-package.py" --ltsc-version $MSVC_VERSION --output "$env:TASK_WORKDIR\vs2022" $VS_EXTRA_PACKAGES

# Compress the Visual Studio SDK
New-Item -ItemType Directory -Path "$env:TASK_WORKDIR\public\build" -Force
#Compress-Archive -Path "$env:TASK_WORKDIR\vs2022" -DestinationPath "$env:TASK_WORKDIR\public\build\visual-studio-2022.zip"
zip -r -9 "$env:TASK_WORKDIR\public\build\visual-studio-2022.zip" vs2022

Write-Output "Build complete, archive created"
