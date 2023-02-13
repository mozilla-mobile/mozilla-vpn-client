# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: ios)

isEmpty(MVPN_DEVELOPMENT_TEAM) {
    MVPN_DEVELOPMENT_TEAM = "43AQ936H96"
}

isEmpty(MVPN_GROUP_ID_IOS) {
    MVPN_GROUP_ID_IOS = "group.org.mozilla.ios.Guardian"
}

adjust {
    message(Adjust SDK enabled)
    DEFINES += MVPN_ADJUST

    SOURCES += \
        apps/vpn/adjust/adjustfiltering.cpp \
        apps/vpn/adjust/adjusthandler.cpp \
        apps/vpn/adjust/adjustproxy.cpp \
        apps/vpn/adjust/adjustproxyconnection.cpp \
        apps/vpn/adjust/adjustproxypackagehandler.cpp \
        apps/vpn/adjust/adjusttasksubmission.cpp

    OBJECTIVE_SOURCES += apps/vpn/platforms/ios/iosadjusthelper.mm

    HEADERS += \
        apps/vpn/adjust/adjustfiltering.h \
        apps/vpn/adjust/adjusthandler.h \
        apps/vpn/adjust/adjustproxy.h \
        apps/vpn/adjust/adjustproxyconnection.h \
        apps/vpn/adjust/adjustproxypackagehandler.h \
        apps/vpn/adjust/adjusttasksubmission.h

    OBJECTIVE_HEADERS += apps/vpn/platforms/ios/iosadjusthelper.h
}

TARGET = "MozillaVPN"

QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.ios
QMAKE_BUNDLE = "FirefoxVPN"

QT += gui-private
QT -= networkauth

CONFIG += c++1z
QMAKE_CXXFLAGS += -Wno-shorten-64-to-32

# For notifications
LIBS += -framework UIKit
LIBS += -framework Foundation
LIBS += -framework StoreKit
LIBS += -framework UserNotifications
LIBS += -framework Network

DEFINES += MZ_IOS

SOURCES += \
    apps/vpn/platforms/macos/macospingsender.cpp \
    apps/vpn/tasks/purchase/taskpurchase.cpp

OBJECTIVE_SOURCES += \
    apps/vpn/platforms/ios/iosiaphandler.mm \
    apps/vpn/platforms/ios/ioscontroller.mm \
    apps/vpn/platforms/ios/iosglue.mm \
    apps/vpn/platforms/ios/iosnetworkwatcher.mm \
    apps/vpn/platforms/ios/iosnotificationhandler.mm \
    apps/vpn/platforms/ios/iosutils.mm

HEADERS += \
    apps/vpn/platforms/macos/macospingsender.h \
    apps/vpn/tasks/purchase/taskpurchase.h

OBJECTIVE_HEADERS += \
    apps/vpn/platforms/ios/iosiaphandler.h \
    apps/vpn/platforms/ios/ioscontroller.h \
    apps/vpn/platforms/ios/iosnetworkwatcher.h \
    apps/vpn/platforms/ios/iosnotificationhandler.h \
    apps/vpn/platforms/ios/iosutils.h

QMAKE_INFO_PLIST= $$PWD/../../../../../ios/app/Info.plist
QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
QMAKE_ASSET_CATALOGS = $$PWD/../../../../../ios/app/Images.xcassets

app_launch_screen.files = $$files(apps/vpn/../../../ios/app/MozillaVPNLaunchScreen.storyboard)
QMAKE_BUNDLE_DATA += app_launch_screen

ios_launch_screen_images.files = $$PWD/../../../../../ios/app/launch.png
QMAKE_BUNDLE_DATA += ios_launch_screen_images

LD_RUNPATH_SEARCH_PATHS.name = "LD_RUNPATH_SEARCH_PATHS"
LD_RUNPATH_SEARCH_PATHS.value = '"$(inherited) @executable_path/../Frameworks"'
QMAKE_MAC_XCODE_SETTINGS += LD_RUNPATH_SEARCH_PATHS

SWIFT_VERSION.name = "SWIFT_VERSION"
SWIFT_VERSION.value = "5.0"
QMAKE_MAC_XCODE_SETTINGS += SWIFT_VERSION

CLANG_ENABLE_MODULES.name = "CLANG_ENABLE_MODULES"
CLANG_ENABLE_MODULES.value = 'YES'
QMAKE_MAC_XCODE_SETTINGS += CLANG_ENABLE_MODULES

SWIFT_OBJC_BRIDGING_HEADER.name = "SWIFT_OBJC_BRIDGING_HEADER"
SWIFT_OBJC_BRIDGING_HEADER.value = 'ios/app/WireGuard-Bridging-Header.h'
QMAKE_MAC_XCODE_SETTINGS += SWIFT_OBJC_BRIDGING_HEADER

MARKETING_VERSION.name = "MARKETING_VERSION"
MARKETING_VERSION.value = $$VERSION
QMAKE_MAC_XCODE_SETTINGS += MARKETING_VERSION

CURRENT_PROJECT_VERSION.name = "CURRENT_PROJECT_VERSION"
CURRENT_PROJECT_VERSION.value = "$$system(echo "$$VERSION" | cut -d. -f1).$$system(date +"%Y%m%d%H%M")"
QMAKE_MAC_XCODE_SETTINGS += CURRENT_PROJECT_VERSION

INFOPLIST_FILE.name = "INFOPLIST_FILE"
INFOPLIST_FILE.value = $$PWD/../../../../../ios/app/Info.plist
QMAKE_MAC_XCODE_SETTINGS += INFOPLIST_FILE

CODE_SIGN_ENTITLEMENTS.name = "CODE_SIGN_ENTITLEMENTS"
CODE_SIGN_ENTITLEMENTS.value = $$PWD/../../../../../ios/app/main.entitlements
QMAKE_MAC_XCODE_SETTINGS += CODE_SIGN_ENTITLEMENTS

CODE_SIGN_IDENTITY.name = "CODE_SIGN_IDENTITY"
CODE_SIGN_IDENTITY.value = 'Apple Development'
QMAKE_MAC_XCODE_SETTINGS += CODE_SIGN_IDENTITY

ENABLE_BITCODE.name = "ENABLE_BITCODE"
ENABLE_BITCODE.value = "NO"
QMAKE_MAC_XCODE_SETTINGS += ENABLE_BITCODE

SDKROOT.name = "SDKROOT"
SDKROOT.value = "iphoneos"
QMAKE_MAC_XCODE_SETTINGS += SDKROOT

SWIFT_PRECOMPILE_BRIDGING_HEADER.name = "SWIFT_PRECOMPILE_BRIDGING_HEADER"
SWIFT_PRECOMPILE_BRIDGING_HEADER.value = "NO"
QMAKE_MAC_XCODE_SETTINGS += SWIFT_PRECOMPILE_BRIDGING_HEADER

SWIFT_OPTIMIZATION_LEVEL.name = "SWIFT_OPTIMIZATION_LEVEL"
SWIFT_OPTIMIZATION_LEVEL.value = "-Onone"
QMAKE_MAC_XCODE_SETTINGS += SWIFT_OPTIMIZATION_LEVEL

MVPN_APP_ID_IOS = "$${QMAKE_TARGET_BUNDLE_PREFIX}.$${QMAKE_BUNDLE}"

APP_ID_IOS.name = "APP_ID_IOS"
APP_ID_IOS.value = "$$MVPN_APP_ID_IOS"
QMAKE_MAC_XCODE_SETTINGS += APP_ID_IOS

GROUP_ID_IOS.name = "GROUP_ID_IOS"
GROUP_ID_IOS.value = "$$MVPN_GROUP_ID_IOS"
QMAKE_MAC_XCODE_SETTINGS += GROUP_ID_IOS

DEVELOPMENT_TEAM.name = "DEVELOPMENT_TEAM"
DEVELOPMENT_TEAM.value = "$$MVPN_DEVELOPMENT_TEAM"
QMAKE_MAC_XCODE_SETTINGS += DEVELOPMENT_TEAM
