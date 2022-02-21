# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script will generate a vsstudio Bundle and md5 hash for it
# This can be uploaded somewhere and then fetched in the fetch/win_vsstudio task

# Why:
# We need a custom install of vsstudio, that is not present in the basic win-image
# We can't install it at runtime, as we need admin rights :c 
# So we need to generate a custom vsstudio bundle, and upload it to a place where it can be fetched


# Note:  Best to run this in a wsb - so your system does not get unneeded installs 
# 

$BUILD_TOOLS_DIR = "C:\MozillaVPNBuild"

if(!(Test-Path $BUILD_TOOLS_DIR)){
    New-Item -Path $BUILD_TOOLS_DIR -ItemType "directory"
    New-Item -Path $BUILD_TOOLS_DIR\.tmp -ItemType "directory"
}

# Step 0: install vc++ tools
if(!(Test-Path $BUILD_TOOLS_DIR\vs_community.exe)){
    Invoke-WebRequest -Uri "https://aka.ms/vs/17/release/vs_community.exe" -OutFile $BUILD_TOOLS_DIR\vs_community.exe
}
if(!(Test-Path "C:\VisualStudio\VC\Auxiliary\Build\vcvars64.bat")){
    Set-Location $BUILD_TOOLS_DIR
    .\vs_community.exe --installPath C:\VisualStudio `
        --add Microsoft.VisualStudio.Workload.CoreEditor `
        --add Microsoft.VisualStudio.Component.VC.Redist.14.Latest `
        --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        --add Microsoft.VisualStudio.Component.VC.CMake.Project `
        --add Microsoft.VisualStudio.Component.VC.Redist.MSM `
        --add Microsoft.VisualStudio.Component.VC.CoreBuildTools `
        --add Microsoft.VisualStudio.Component.VC.CoreIde `
        --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
        --passive --norestart
    if($LASTEXITCODE  -ne 0){
        Write-Output "Failed to trigger install of vs-studio?"
        Write-Output "Reason " 
        Write-Output $LASTEXITCODE
        Exit-PSSession 1
    }
    #Write-Output "Failed to install VSCODE?"
    Wait-Process -Name "vs_community","vs_setup_bootstrapper" # They will start setup.exe
    Wait-Process -Name "setup" # this is the actuall install
}

Copy-Item -Recurse -Path "C:\Program Files (x86)\Windows Kits\10\*" -Destination C:\VisualStudio\SDK\
