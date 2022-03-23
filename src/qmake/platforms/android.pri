# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

message(Platform: android)

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

    HEADERS += \
       adjust/adjustfiltering.h \
       adjust/adjusthandler.h \
       adjust/adjustproxy.h \
       adjust/adjustproxyconnection.h \
       adjust/adjustproxypackagehandler.h \
       adjust/adjusttasksubmission.h
}

versionAtLeast(QT_VERSION, 5.15.1) {
  QMAKE_CXXFLAGS *= -Werror
}

versionAtLeast(QT_VERSION, 6.0.0) {
    # We need to include qtprivate api's
    # As QAndroidBinder is not yet implemented with a public api
    QT+=core-private
}


# Android Deploy-to-Qt strips the info anyway
# but we want to create an extra bundle with the info :)
CONFIG += force_debug_info
CONFIG += c++14

TARGET = mozillavpn

QT += qml
QT += xml
LIBS += \-ljnigraphics\

!versionAtLeast(QT_VERSION, 6.0.0) {
    QT += androidextras
}

DEFINES += MVPN_ANDROID

INCLUDEPATH += platforms/android

SOURCES +=  \
    platforms/android/androidauthenticationlistener.cpp \
    platforms/android/androidcontroller.cpp \
    platforms/android/androidiaphandler.cpp \
    platforms/android/androidnotificationhandler.cpp \
    platforms/android/androidutils.cpp \
    platforms/android/androidwebview.cpp \
    platforms/android/androidvpnactivity.cpp \
    platforms/android/androiddatamigration.cpp \
    platforms/android/androidappimageprovider.cpp \
    platforms/android/androidapplistprovider.cpp \
    platforms/android/androidsharedprefs.cpp \
    tasks/authenticate/desktopauthenticationlistener.cpp \
    tasks/purchase/taskpurchase.cpp

HEADERS +=  \
    platforms/android/androidauthenticationlistener.h \
    platforms/android/androidcontroller.h \
    platforms/android/androidiaphandler.h \
    platforms/android/androidnotificationhandler.h \
    platforms/android/androidutils.h \
    platforms/android/androidwebview.h \
    platforms/android/androidvpnactivity.h \
    platforms/android/androiddatamigration.h\
    platforms/android/androidappimageprovider.h \
    platforms/android/androidapplistprovider.h \
    platforms/android/androidsharedprefs.h \
    platforms/android/androidjnicompat.h \
    tasks/authenticate/desktopauthenticationlistener.h \
    tasks/purchase/taskpurchase.h


# Usable Linux Imports
SOURCES += \
    platforms/linux/linuxpingsender.cpp \
    platforms/linux/linuxcryptosettings.cpp

HEADERS += \
    platforms/linux/linuxpingsender.h

# We need to compile our own openssl :/
exists($$PWD/../../../3rdparty/openSSL/openssl.pri) {
   include($$PWD/../../../3rdparty/openSSL/openssl.pri)
} else {
   message(Have you imported the 3rd-party git submodules? Read the README.md)
   error(Did not found openSSL in 3rdparty/openSSL - Exiting Android Build)
}

# For the android build we need to unset those
# Otherwise the packaging will fail 🙅
OBJECTS_DIR =
MOC_DIR =
RCC_DIR =
UI_DIR =

DISTFILES += \
    ../android/AndroidManifest.xml \
    ../android/build.gradle \
    ../android/gradle/wrapper/gradle-wrapper.jar \
    ../android/gradle/wrapper/gradle-wrapper.properties \
    ../android/gradlew \
    ../android/gradlew.bat \
    ../android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../../../android
