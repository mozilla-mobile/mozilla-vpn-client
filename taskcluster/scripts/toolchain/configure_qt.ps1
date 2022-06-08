# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


# Usage: ./configure_qt.ps1
# Writes a QT.conf to make qmake find the correct Qt installation
# This is moved into the QT directory during the taskcluster build

$LIB_PATH = (resolve-path "$PSScriptRoot/lib").ToString().Replace("\","/")
$PREFIX_PATH = (resolve-path "$PSScriptRoot").ToString().Replace("\","/")



$conf = "[Paths]
Prefix=$PREFIX_PATH
Libraries=$LIB_PATH
"

Write-Output $conf |  Out-File -Encoding Utf8 -FilePath  $PSScriptRoot/bin/qt.conf

Write-Output "Written $PSScriptRoot/bin/qt.conf"
Write-Output "Mapping QT Prefix to $PREFIX_PATH"

