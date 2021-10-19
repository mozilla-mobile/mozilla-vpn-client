#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

parentDir=$(dirname $0)
. "$parentDir/commons.sh"

print N "This script bakes our custom shaders"
print N ""

repoDir="$(dirname "$parentDir")"
shaderSourceDir="$repoDir/src/ui/resources/shaders/src"
shaderDistDir="$repoDir/src/ui/resources/shaders/baked"

rm -rf "$shaderDistDir"/* || die
totalCount=$(find "$shaderSourceDir" -type f | wc -l | xargs)

if [[ $totalCount -eq 0 ]]; then
  print N "No shader files found."
  exit 0
fi

for inputFilePath in "$shaderSourceDir"/*
do
  filename=$(basename "$inputFilePath")
  outputFilePath="$shaderDistDir"/"$filename".qsb

  # Generate shader with qsb
  # For options and arguments see QSB Manual:
  # https://doc.qt.io/qt-6/qtshadertools-qsb.html
  command -v qsb &>/dev/null || die "qsb not found"
  qsb --glsl "100 es,120,150" --hlsl 50 --msl 12 -o "$outputFilePath" "$inputFilePath" || die "Failed to generate the shader file"
done

print N "Done baking shaders!"
