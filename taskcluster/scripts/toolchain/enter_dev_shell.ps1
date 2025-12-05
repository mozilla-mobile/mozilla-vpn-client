# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

#TODO: PUT THIS INTO THE VSSTUDIO ZIP:#
# Is in the VS_bundle used by fetch/vs-dev-env. 
# Elevates a powershell env to a vs-studio "dev-console" equivalent. 

$VS_TARGET_ARCH = switch ($PROCESSOR_ARCHITECTURE) {
  "amd64"   { "x64"   }
  "x64"     { "x64"   }
  "arm64"   { "arm64" }
  "aarch64" { "arm64" }
  default   { "x64"   }
}

$VS_STUDIO_LOCATION =resolve-path $PSScriptRoot
$MSVC_VERSION =(Get-ChildItem -Path "$VS_STUDIO_LOCATION\VC\Tools\MSVC" | Select-Object -First 1)

if (Test-Path "$PSScriptRoot\Windows Kits") {
  $W10_SDK_PATH = resolve-path "$PSScriptRoot\Windows Kits\10"
} else {
  $W10_SDK_PATH = resolve-path "$PSScriptRoot\SDK"
}
$W10_SDK_VERSION =(Get-ChildItem -Path "$W10_SDK_PATH\include" | Select-Object -First 1)

$INCLUDE_ADDS =   `
                  "$W10_SDK_PATH\include\$W10_SDK_VERSION\ucrt;",`
                  "$W10_SDK_PATH\include\$W10_SDK_VERSION\shared;",`
                  "$W10_SDK_PATH\include\$W10_SDK_VERSION\um;",`
                  "$W10_SDK_PATH\include\$W10_SDK_VERSION\winrt;",`
                  "$W10_SDK_PATH\include\$W10_SDK_VERSION\cppwinrt;" ,`
                  "$VS_STUDIO_LOCATION\DIA SDK\include\;",`
                  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\include;",`
                  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\include;"

$ENV:INCLUDE =""
ForEach-Object -InputObject $INCLUDE_ADDS {
  $ENV:INCLUDE +=$_
}

$LIB_ADDS = `
    "$W10_SDK_PATH\Lib\$W10_SDK_VERSION\ucrt\$VS_TARGET_ARCH;" ,`
    "$W10_SDK_PATH\Lib\$W10_SDK_VERSION\um\$VS_TARGET_ARCH;" ,`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\lib\$VS_TARGET_ARCH;" ,`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\lib\$VS_TARGET_ARCH;" ,`
    "$VS_STUDIO_LOCATION\DIA SDK\lib\$PROCESSOR_ARCHITECTURE;"

$ENV:LIB =""
ForEach-Object -InputObject $LIB_ADDS {
  $ENV:LIB +=$_
}

$LIBPATH_ADDS =  `
  "$W10_SDK_PATH\UnionMetadata\$W10_SDK_VERSION;" ,`
  "$W10_SDK_PATH\References\$W10_SDK_VERSION;" ,`
  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\lib\$VS_TARGET_ARCH;" ,`
  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\lib\$VS_TARGET_ARCH;" 

$ENV:LIBPATH =""
ForEach-Object -InputObject $LIBPATH_ADDS {
    $ENV:LIBPATH +=$_
}

$PATH_ADDS = ";",`
  "$VS_STUDIO_LOCATION\DIA SDK\bin\$PROCESSOR_ARCHITECTURE;" ,`
  "$W10_SDK_PATH\bin\$W10_SDK_VERSION\$VS_TARGET_ARCH;" ,`
  "$W10_SDK_PATH\bin\$VS_TARGET_ARCH;",`
  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\bin\HostX64\$VS_TARGET_ARCH;",`
  "$VS_STUDIO_LOCATION\Common7\IDE\VC\VCPackages;",`
  "$VS_STUDIO_LOCATION\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer;",`
  "$VS_STUDIO_LOCATION\MSBuild\Current\Bin\$PROCESSOR_ARCHITECTURE;",`
  "$VS_STUDIO_LOCATION\Common7\IDE;",`
  "$VS_STUDIO_LOCATION\Common7\Tools;",`
  "$VS_STUDIO_LOCATION\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;",`
  "$VS_STUDIO_LOCATION\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;"

ForEach-Object -InputObject $PATH_ADDS {
    $ENV:PATH+=$_
}
# Doing Powershell += apparently adds a whitespeace 
# e.g. Path"...some.exe; C:/next.exe" which breaks cmd
$env:PATH= $ENV:PATH -replace("; ",";")
$env:LIBPATH= $ENV:LIBPATH -replace("; ",";")
$env:LIB= $ENV:LIB -replace("; ",";")
$env:INCLUDE= $ENV:INCLUDE -replace("; ",";")

$ENV:UCRTVersion=$W10_SDK_VERSION
$ENV:WindowsSDKLibVersion=$W10_SDK_VERSION
$ENV:WindowsSDKVersion=$W10_SDK_VERSION
$ENV:WindowsSdkVerBinPath="$W10_SDK_PATH\bin\$W10_SDK_PATH"



write-host "Registered Windows SDK: $W10_SDK_VERSION"
