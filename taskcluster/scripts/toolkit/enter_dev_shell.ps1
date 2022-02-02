# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Eleveates a Powershell session into a Dev-Console session
# Searches for a vs-studio install in $DEFAULT_PATHS
# Takes the first, records any ENV changes 
# and apply it into the current session

#Paths where we can look for VSStudio
$DEFAULT_PATHS =  "C:\$env:VSSTUDIO_PATH", `
                  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional", `
                  "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community", `
                  "C:\Program Files (x86)\Microsoft Visual Studio\2021\Professional", `
                  "C:\Program Files (x86)\Microsoft Visual Studio\2021\Community"


foreach ($VS_STUDIO_PATH in $DEFAULT_PATHS) {
  "Checking: $VS_STUDIO_PATH"
  $VC_VARS_PATH = $VS_STUDIO_PATH+"\VC\Auxiliary\Build\vcvars64.bat"
  if(Test-Path $VC_VARS_PATH){
    "Found $VC_VARS_PATH"
    cmd.exe /c "call `"$VC_VARS_PATH`" && set > %temp%\vcvars.txt"
    Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
      if ($_ -match "^(.*?)=(.*)$") {
          Set-Content "env:\$($matches[1])" $matches[2]
      }
    }
    write-host "`nVisual Studio 2019 Command Prompt variables set. ($VC_VARS_PATH)" -ForegroundColor Yellow
    return;
  }        
}