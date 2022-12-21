#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
if [ -f .env ]; then
  . .env
fi
# This script will check all "Staged" QML files and format them c: 
$QT_HOST_PATH/bin/qmlformat -i -f $(git diff --name-only --cached | grep ".qml") 

# QML Format will save a .qml~ as backup. we have no need for that clutter c: 
find . -type f -name "*.qml~" -delete
