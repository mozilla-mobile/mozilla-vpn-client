#!/bin/sh

#  ci_post_clone.sh
#  MozillaVPN
#
#  Created by mozilla on 29.07.21.
#  

cd /Volumes/workspace/repository

git clone https://github.com/mbirghan/qt_ios_build

gem install xcodeproj -v 0.28.2

pip3 install "glean_parser==3.5"
pip3 install pyhumps
pip3 install pyyaml
python3 scripts/generate_glean.py

brew install go

export QT_IOS_BIN=`pwd`/qt_ios_build/ios/bin
export PATH=`pwd`/qt_ios_build/ios/bin:$PATH

git submodule update --remote --depth 1 i18n
python3 scripts/importLanguages.py -m

./scripts/apple_compile.sh ios
