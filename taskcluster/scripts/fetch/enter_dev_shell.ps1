# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Eleveates a Powershell session into a Dev-Console session
# Searches for a vs-studio install in $DEFAULT_PATHS
# Takes the first, records any ENV changes 
# and apply it into the current session


#TODO: PUT THIS INTO THE VSSTUDIO ZIP:#

#Paths where we can look for VSStudio
echo($PSScriptRoot)


$VS_STUDIO_LOCATION =resolve-path $PSScriptRoot
$W10_SDK_PATH =  resolve-path "$PSScriptRoot\SDK"
$W10_SDK_VERSION = "10.0.19041.0"
$MSVC_VERSION ="14.30.30705"


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
    "$W10_SDK_PATH\lib\$W10_SDK_VERSION\ucrt\x64;" ,`
    "$W10_SDK_PATH\lib\$W10_SDK_VERSION\um\x64;" ,`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\lib\x64;" ,`
    "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\lib\x64;" ,`
    "$VS_STUDIO_LOCATION\DIA SDK\lib\amd64;"
    
$ENV:LIB =""
ForEach-Object -InputObject $LIB_ADDS {
  $ENV:LIB +=$_
}

$LIBPATH_ADDS =  `
  "$W10_SDK_PATH\UnionMetadata\$W10_SDK_VERSION;" ,`
  "$W10_SDK_PATH\References\$W10_SDK_VERSION;" ,`
  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\ATLMFC\lib\x64;" ,`
  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\lib\x64;" 

$ENV:LIBPATH =""
ForEach-Object -InputObject $LIBPATH_ADDS {
    $ENV:LIBPATH +=$_
}

$PATH_ADDS = ";",`
  "$VS_STUDIO_LOCATION\DIA SDK\bin\amd64;" ,`
  "$W10_SDK_PATH\bin\$W10_SDK_VERSION\x64;" ,`
  "$W10_SDK_PATH\bin\x64;",`
  "$VS_STUDIO_LOCATION\VC\Tools\MSVC\$MSVC_VERSION\bin\HostX64\x64;",`
  "$VS_STUDIO_LOCATION\Common7\IDE\VC\VCPackages;",`
  "$VS_STUDIO_LOCATION\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer;",`
  "$VS_STUDIO_LOCATION\MSBuild\Current\Bin\amd64;",`
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
