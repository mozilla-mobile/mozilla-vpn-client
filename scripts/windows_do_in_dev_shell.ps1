# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Eleveates a Powershell session into a Dev-Console session
# and runs the provided arg

Write-Host "Enter-DevShell"
invoke-expression -Command $PSScriptRoot\windows_enter_dev_shell.ps1

if($args.count -eq 1){
  invoke-expression -Command $args[0]
  exit 1;
}
Write-Host "Usage $ ./windows_do_in_dev_shell.ps1 Write-host 'hello from the dev shell' "