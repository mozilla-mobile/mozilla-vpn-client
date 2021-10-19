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

fileCounter=0
for inputFilePath in "$shaderSourceDir"/*
do
  fileCounter=$[$fileCounter + 1]
  filename=$(basename "$inputFilePath")
  outputFilePath="$shaderDistDir"/"$filename".qsb
  shaderLabel="Shader $fileCounter/$totalCount"

  # Generate shader with qsb
  # For options and arguments see QSB Manual:
  # https://doc.qt.io/qt-6/qtshadertools-qsb.html
  if qsb --glsl "100 es,120,150" --hlsl 50 --msl 12 -o "$outputFilePath" "$inputFilePath"; then
    print G "$shaderLabel (baked): $filename"
    print N "$inputFilePath -> $outputFilePath"
  else
    print R "$shaderLabel (failed): $filename"
    print N "$inputFilePath"
  fi
done

print N ""
print N "Done baking shaders!"
