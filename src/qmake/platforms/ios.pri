# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: ios)

adjust {
    message(Adjust SDK enabled)
    DEFINES += MVPN_ADJUST

    SOURCES += \
        adjust/adjustfiltering.cpp \
        adjust/adjusthandler.cpp \
        adjust/adjustproxy.cpp \
        adjust/adjustproxyconnection.cpp \
        adjust/adjustproxypackagehandler.cpp \
        adjust/adjusttasksubmission.cpp

    OBJECTIVE_SOURCES += platforms/ios/iosadjusthelper.mm

    HEADERS += \
        adjust/adjustfiltering.h \
        adjust/adjusthandler.h \
        adjust/adjustproxy.h \
        adjust/adjustproxyconnection.h \
        adjust/adjustproxypackagehandler.h \
        adjust/adjusttasksubmission.h

    OBJECTIVE_HEADERS += platforms/ios/iosadjusthelper.h
}

TARGET = "Mozilla VPN"

QMAKE_TARGET_BUNDLE_PREFIX = org.mozilla.ios
QT += gui-private
QT -= networkauth

CONFIG += c++1z

# For the authentication
LIBS += -framework AuthenticationServices

# For notifications
LIBS += -framework UIKit
LIBS += -framework Foundation
LIBS += -framework StoreKit
LIBS += -framework UserNotifications

DEFINES += MVPN_IOS

SOURCES += \
    platforms/macos/macospingsender.cpp \
    tasks/purchase/taskpurchase.cpp

OBJECTIVE_SOURCES += \
    platforms/ios/iosiaphandler.mm \
    platforms/ios/iosauthenticationlistener.mm \
    platforms/ios/ioscontroller.mm \
    platforms/ios/iosdatamigration.mm \
    platforms/ios/iosglue.mm \
    platforms/ios/iosnotificationhandler.mm \
    platforms/ios/iosutils.mm \
    platforms/macos/macoscryptosettings.mm

HEADERS += \
    features/featureioskillswitch.h \
    platforms/macos/macospingsender.h \
    tasks/purchase/taskpurchase.h

OBJECTIVE_HEADERS += \
    platforms/ios/iosiaphandler.h \
    platforms/ios/iosauthenticationlistener.h \
    platforms/ios/ioscontroller.h \
    platforms/ios/iosdatamigration.h \
    platforms/ios/iosnotificationhandler.h \
    platforms/ios/iosutils.h

QMAKE_INFO_PLIST= $$PWD/../../../ios/app/Info.plist
QMAKE_ASSET_CATALOGS_APP_ICON = "AppIcon"
QMAKE_ASSET_CATALOGS = $$PWD/../../../ios/app/Images.xcassets

app_launch_screen.files = $$files($$PWD/../../../ios/app/MozillaVPNLaunchScreen.storyboard)
QMAKE_BUNDLE_DATA += app_launch_screen

ios_launch_screen_images.files = $$PWD/../../../ios/app/launch.png
QMAKE_BUNDLE_DATA += ios_launch_screen_images

