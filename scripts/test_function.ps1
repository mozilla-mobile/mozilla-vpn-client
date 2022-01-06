# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function runTest{
  param ($TestName)
  npm run functionalTest $TestName
}

if($args.count -eq 1){
  runTest($args[0])
  exit 1;
}

foreach ($test in Get-ChildItem -Path "tests/functional/test*") {
  runTest($test)
}
