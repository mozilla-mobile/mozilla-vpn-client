#!/bin/sh

#  ci_post_clone.sh
#  MozillaVPN
#
#  Created by mozilla on 29.07.21.
#

cd /Volumes/workspace/repository

git submodule init
git submodule update

git clone https://github.com/mbirghan/qt_ios_build
cd qt_ios_build
cat x* > qt_static.tar.gz
tar xf qt_static.tar.gz
cd ..

export QT_IOS_BIN=`pwd`/qt_ios_build/ios/bin

export PATH=`pwd`/qt_ios_build/ios/bin:/Users/local/.gem/ruby/2.6.0/bin:/Users/local/Library/Python/3.8/bin:$PATH
gem install xcodeproj --user-install

pip3 install "glean_parser==3.5" --user
pip3 install pyhumps --user
pip3 install pyyaml --user
python3 scripts/generate_glean.py

brew install go
# wget -nc https://golang.org/dl/go1.16.9.darwin-amd64.tar.gz
# tar -xzf go1.16.9.darwin-amd64.tar.gz
# export PATH=`pwd`/go/bin:$PATH
# export GOROOT=`pwd`/go
# export GOPATH=`pwd`

python3 scripts/importLanguages.py -m

echo "DEVELOPMENT_TEAM = 43AQ936H96" >> xcode.xconfig
echo "GROUP_ID_MACOS = group.org.mozilla.macos.Guardian" >> xcode.xconfig
echo "APP_ID_MACOS = org.mozilla.macos.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_MACOS = org.mozilla.macos.FirefoxVPN.network-extension" >> xcode.xconfig
echo "LOGIN_ID_MACOS = org.mozilla.macos.FirefoxVPN.login-item" >> xcode.xconfig
echo "GROUP_ID_IOS = group.org.mozilla.ios.Guardian" >> xcode.xconfig
echo "APP_ID_IOS = org.mozilla.ios.FirefoxVPN" >> xcode.xconfig
echo "NETEXT_ID_IOS = org.mozilla.ios.FirefoxVPN.network-extension" >> xcode.xconfig

./scripts/apple_compile.sh ios

rm ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "<plist version=\"1.0\">" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "<dict>" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "	<key>IDEWorkspaceSharedSettings_AutocreateContextsIfNeeded</key>" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "	<false/>" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "</dict>" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings
echo "</plist>" >> ./MozillaVPN.xcodeproj/project.xcworkspace/xcshareddata/WorkspaceSettings.xcsettings

rm ./MozillaVPN.xcodeproj/xcshareddata/xcschemes/MozillaVPN.xcscheme
mv MozillaVPN.xcscheme ./MozillaVPN.xcodeproj/xcshareddata/xcschemes/

make -f MozillaVPN.xcodeproj/qt_makeqmake.mak
make -f MozillaVPN.xcodeproj/qt_preprocess.mak
