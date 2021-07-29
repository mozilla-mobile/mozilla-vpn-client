#!/bin/sh

#  ci_post_clone.sh
#  MozillaVPN
#
#  Created by mozilla on 29.07.21.
#  

cd /Volumes/workspace/repository

https://github.com/mbirghan/qt_ios_build

pip3 install "glean_parser==3.5"
pip3 install pyhumps
pip3 install pyyaml
python3 scripts/generate_glean.py

export QT_IOS_BIN=`pwd`/qt_ios_build/ios/bin

./scripts/apple_compile.sh ios
