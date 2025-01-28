# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Elevates a PowerShell session into a Dev-Console session
# Searches for a vs-studio install in $DEFAULT_PATHS
# Takes the first, records any ENV changes
# and apply it into the current session

# Paths where we can look for VSStudio
$VS_STUDIO_LOCATION = resolve-path $PSScriptRoot
$W10_SDK_PATH = resolve-path "$PSScriptRoot\Windows Kits\10"
$W10_SDK_VERSION = "10.0.19041.0"
$MSVC_VERSION = "14.42.34433"

param (
    [string]$ARCH = "x64"
)

$missingPaths = @()

function AddPaths {
    param (
        [string[]]$Paths
        
    )

    $result = ""
    foreach ($path in $Paths) {
        if (Test-Path $path) {
            $result += "$path;"
        } else {
            Write-Host "The following path was not found: $path" -ForegroundColor Yellow
            $missingPaths += $path
        }
    }
    return $result -replace "; $", ""
}

$INCLUDE_ADDS =   `
    "$W10_SDK_PATH\include\$W10_SDK_VERSION\ucrt",`
    "$W10_SDK_PATH\include\$W10_SDK_VERSION\shared",`
    "$W10_SDK_PATH\include\$W10_SDK_VERSION\um",`
    "$W10_SDK_PATH\include\$W10_SDK_VERSION\winrt",`
    "$W10_SDK_PATH\include\$W10_SDK_VERSION\cppwinrt",`
    "$VS_STUDIO_LOCATION\DIA SDK\include",`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\include",`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\include"

$ENV:INCLUDE = AddPaths -Paths $INCLUDE_ADDS

$LIB_ADDS = `
    "$W10_SDK_PATH\lib\$W10_SDK_VERSION\ucrt\$ARCH",`
    "$W10_SDK_PATH\lib\$W10_SDK_VERSION\um\$ARCH",`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\lib\$ARCH",`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\lib\$ARCH",`
    "$VS_STUDIO_LOCATION\DIA SDK\lib\amd64"

$ENV:LIB=AddPaths -Paths $LIB_ADDS

$LIBPATH_ADDS =  `
    "$W10_SDK_PATH\UnionMetadata\$W10_SDK_VERSION",`
    "$W10_SDK_PATH\References\$W10_SDK_VERSION",`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\lib\$ARCH",`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\lib\$ARCH"

$ENV:LIBPATH=AddPaths -Paths $LIBPATH_ADDS

$PATH_ADDS = `
    "$VS_STUDIO_LOCATION\DIA SDK\bin\amd64",`
    "$W10_SDK_PATH\bin\$W10_SDK_VERSION\$ARCH",`
    "$W10_SDK_PATH\bin\$ARCH",`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\bin\Hostx64\$ARCH",`
    "$VS_STUDIO_LOCATION\MSBuild\Current\Bin\amd64"

# Do not overwrite the existing PATH; append to it instead
$ENV:PATH = $ENV:PATH+";"+ (AddPaths -Paths $PATH_ADDS)

$ENV:UCRTVersion = $W10_SDK_VERSION
$ENV:WindowsSDKLibVersion = $W10_SDK_VERSION
$ENV:WindowsSDKVersion = $W10_SDK_VERSION
$ENV:WindowsSdkVerBinPath = "$W10_SDK_PATH\bin\$W10_SDK_PATH"

# Report missing paths
if ($missingPaths.Count -gt 0) {
    Write-Host "The following paths were not found:" -ForegroundColor Yellow
    $missingPaths | ForEach-Object { Write-Host $_ -ForegroundColor Red }
} else {
    Write-Host "All paths were found and added successfully." -ForegroundColor Green
}

Write-Host "Registered Windows SDK: $W10_SDK_VERSION"
